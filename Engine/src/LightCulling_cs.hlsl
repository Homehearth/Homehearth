#include "Common.hlsli"

//
// ForwardPlus: https://www.3dgep.com/forward-plus/
//
// The purpose of the light culling compute shader is to
// update the global light index list and the light grid
// that is required by the fragment shader.
// Two lists need to be updated per frame:
//		- Light index list for opaque geometry.
//		- Light index list for transparent geometry

// Group shared variables.
groupshared uint uMinDepth;
groupshared uint uMaxDepth;
groupshared Frustum GroupFrustum;

// Opaque geometry light lists.

// To keep track of the number of lights that
// are intersecting the current tile frustum.
groupshared uint o_LightCount;

// Offset into the global light index list.
// This index will be written to the light grid and
// is used as the starting offset when copying the
// local light index list to global light index list.
groupshared uint o_LightIndexStartOffset;
groupshared uint o_LightList[MAX_LIGHTS];

// Transparent geometry light lists.
groupshared uint t_LightCount;
groupshared uint t_LightIndexStartOffset;
groupshared uint t_LightList[MAX_LIGHTS];

// Add the light to the visible light list for opaque geometry.
void o_AppendLight(uint lightIndex)
{
	uint index; // Index into the visible lights array.
	InterlockedAdd(o_LightCount, 1, index);
	if (index < MAX_LIGHTS)
	{
		o_LightList[index] = lightIndex;
	}
}

// Add the light to the visible light list for transparent geometry.
void t_AppendLight(uint lightIndex)
{
	uint index; // Index into the visible lights array.
	InterlockedAdd(t_LightCount, 1, index);
	if (index < MAX_LIGHTS)
	{
		t_LightList[index] = lightIndex;
	}
}

[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void main(ComputeShaderIn input)
{
	// Calculate min & max depth in threadgroup / tile.
	// Each thread in the thread group will sample the
	// depth buffer only once for the current thread and
	// thus all threads in a group will sample all depth
	// values for a single tile.

	int2 texCoord = input.dispatchThreadID.xy;
	float fDepth = t_depth.Load(int3(texCoord, 0)).r;

	//float zNear = 40.f;
	//float zFar = 220.f;

	// Linearize the depth value from depth buffer (must do this because we created it using projection)
	//fDepth = (0.5 * c_projection[3][2]) / (fDepth + 0.5 * c_projection[2][2] - 0.5);
	
	//fDepth = ((2.0f * zNear) / (zFar + zNear - fDepth * (zFar - zNear)));

	// atomic operations only work on integers,
	// hence we reinterrpret the bits from the
	// floating-point depth as an unsigned integer.
	// Since we expect all depth values in the depth
	// map to be stored in the range [0..1].
	uint uDepth = asuint(fDepth);

	// Since we are setting group-shared variables,
	// only one thread in the group needs to set them.
	if (input.groupIndex == 0)
	{
		uMinDepth = 0xffffffff; // FLT_MAX as a uint
		uMaxDepth = 0;
		o_LightCount = 0;
		t_LightCount = 0;
		GroupFrustum = in_Frustums[input.groupID.x + (input.groupID.y * numThreadGroups.x)];
	}

	// Blocks execution of all threads in a group until
	// all group shared accesses have been completed and
	// all threads in the group have reached this call.
    GroupMemoryBarrierWithGroupSync();

	// The InterlockedMin and InterlockedMax methods
	// are used to atomically update the uMinDepth and
	// uMaxDepth group - shared variables based on the current threads depth value.
    InterlockedMin(uMinDepth, uDepth);	// Performs a guaranteed atomic min.
	InterlockedMax(uMaxDepth, uDepth);	// Performs a guaranteed atomic max.
	
    GroupMemoryBarrierWithGroupSync();

	// After the minimum and maximum depth values for
	// the current tile have been found, we can reinterrpret
	// the unsigned integer back to a float so that we can use
	// it to compute the view space clipping planes for the current tile.
    float fMinDepth = asfloat(uMinDepth);
    float fMaxDepth = asfloat(uMaxDepth);

    // Convert depth values to view space.
    float minDepthVS = ClipToView(float4(0, 0, fMinDepth, 1)).z;
    float maxDepthVS = ClipToView(float4(0, 0, fMaxDepth, 1)).z;

	// When culling lights for transparent geometry, we don’t want to use
	// the minimum depth value from the depth map. Instead we will clip the
	// lights using the camera’s near clipping plane. In this case, we will
	// use the nearClipVS value which is the distance to the camera’s near
	// clipping plane in view space.
    float nearClipVS = ClipToView(float4(0, 0, 0, 1)).z;

    // Clipping plane for minimum depth value 
    // (used for testing lights within the bounds of opaque geometry).
	// assuming left-handed coord.
    Plane minPlane = { float3(0, 0, 1), minDepthVS };

	Frustum frustum = in_Frustums[input.groupID.x + (input.groupID.y * numThreadGroups.x)];
    // Each thread in a group will cull 1 light until all lights have been culled.
	for (uint i = input.groupIndex; i < c_info.x; i += TILE_SIZE * TILE_SIZE)
	{
		if (sb_lights[i].enabled)
		{
			Light light = sb_lights[i];

			switch (light.type)
			{
				case DIRECTIONAL_LIGHT:
				{
					t_AppendLight(i);
					o_AppendLight(i);
				}
				break;
				case POINT_LIGHT:
				{
					Sphere sphere = { light.positionVS.xyz, light.range };
					if (SphereInsideFrustum(sphere, frustum, nearClipVS, maxDepthVS))
					{
						// Add light to light list for transparent geometry.
						t_AppendLight(i);

						if (!SphereInsidePlane(sphere, minPlane))
						{
							// Add light to light list for opaque geometry.
							o_AppendLight(i);
						}
					}
				}
				break;
				default:
				break;
			}
		}
	}

	// Wait till all threads in group have caught up.
	GroupMemoryBarrierWithGroupSync();

	// Update global memory with visible light buffer.
	// First update the light grid (only thread 0 in group needs to do this)
	if (input.groupIndex == 0)
	{
		// InterlockedAdd guarantees that the group - shared light count variabl is only updated by a single thread at a time.
		// This way we avoid any race conditions that may occur when multiple threads try to increment the group-shared light count at the same time.

		// Update light grid for opaque geometry.
		InterlockedAdd(o_LightIndexCounter[0], o_LightCount, o_LightIndexStartOffset);
		o_LightGrid[input.groupID.xy] = uint2(o_LightIndexStartOffset, o_LightCount);

		// Update light grid for transparent geometry.
		InterlockedAdd(t_LightIndexCounter[0], t_LightCount, t_LightIndexStartOffset);
		t_LightGrid[input.groupID.xy] = uint2(t_LightIndexStartOffset, t_LightCount);
	}

	GroupMemoryBarrierWithGroupSync();

	// Now update the light index list (all threads).
	// For opaque goemetry.
	for (uint i = input.groupIndex; i < o_LightCount; i += TILE_SIZE * TILE_SIZE)
	{
		o_LightIndexList[o_LightIndexStartOffset + i] = o_LightList[i];
	}
	// For transparent geometry.
	for (uint i = input.groupIndex; i < t_LightCount; i += TILE_SIZE * TILE_SIZE)
	{
		t_LightIndexList[t_LightIndexStartOffset + i] = t_LightList[i];
	}

	// Update the debug texture output.
	if (input.groupThreadID.x == 0 || input.groupThreadID.y == 0)
	{
		rw_heatMap[texCoord] = float4(0, 0, 0, 0.9f);
	}
	else if (input.groupThreadID.x == 1 || input.groupThreadID.y == 1)
	{
		rw_heatMap[texCoord] = float4(1, 1, 1, 0.5f);
	}
	else if (o_LightCount > 0)
	{
		float normalizedLightCount = o_LightCount / 10.0f;
		float4 lightCountHeatMapColor = t_LightCountHeatMap.SampleLevel(s_linearClamp, float2(normalizedLightCount, 0), 0);
		rw_heatMap[texCoord] = lightCountHeatMapColor;
	}
	else
	{
		rw_heatMap[texCoord] = float4(0, 0, 0, 1);
	}
}