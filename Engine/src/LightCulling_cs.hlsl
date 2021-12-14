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
groupshared uint group_uMinDepth;
groupshared uint group_uMaxDepth;
groupshared Frustum group_GroupFrustum; // COULD THIS BE THE ERROR !?!?!?

// To keep track of the number of lights that
// are intersecting the current tile frustum.
groupshared uint group_opaq_LightCount;

// Offset into the global light index list.
// This index will be written to the light grid and
// is used as the starting offset when copying the
// local light index list to global light index list.
groupshared uint group_opaq_LightIndexStartOffset;
groupshared uint group_opaq_LightList[MAX_LIGHTS];

groupshared uint group_trans_LightCount;
groupshared uint group_trans_LightIndexStartOffset;
groupshared uint group_trans_LightList[MAX_LIGHTS];

void AddLightToOpaqueList(uint lightIndex)
{
    uint index; // Index into the visible lights array.
    InterlockedAdd(group_opaq_LightCount, 1, index);
    if (index < MAX_LIGHTS)
    {
        group_opaq_LightList[index] = lightIndex;
    }
}

void AddLightToTransparentList(uint lightIndex)
{
    uint index; // Index into the visible lights array.
    InterlockedAdd(group_trans_LightCount, 1, index);
    if (index < MAX_LIGHTS)
    {
        group_trans_LightList[index] = lightIndex;
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
	float z_b = t_depth.Load(int3(texCoord, 0)).r;
	//float z_n = 2.0 * z_b - 1.0;

	//float zNear = 40.0f;
	//float zFar = 220.0f;

	//// Linear Depth.
	//float z_w = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));

	float fDepth = z_b;

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
        group_uMinDepth = 0xffffffff; // FLT_MAX as a uint
		group_uMaxDepth = 0;
		group_opaq_LightCount = 0;
		group_trans_LightCount = 0;
		group_GroupFrustum = sb_frustums_in[input.groupID.x + (input.groupID.y * numThreadGroups.x)];
    }

	// Blocks execution of all threads in a group until
	// all group shared accesses have been completed and
	// all threads in the group have reached this call.
    GroupMemoryBarrierWithGroupSync();

	// The InterlockedMin and InterlockedMax methods
	// are used to atomically update the uMinDepth and
	// uMaxDepth group - shared variables based on the current threads depth value.
    InterlockedMin(group_uMinDepth, uDepth);	// Performs a guaranteed atomic min.
	InterlockedMax(group_uMaxDepth, uDepth);	// Performs a guaranteed atomic max.
	
    GroupMemoryBarrierWithGroupSync();

	// After the minimum and maximum depth values for
	// the current tile have been found, we can reinterrpret
	// the unsigned integer back to a float so that we can use
	// it to compute the view space clipping planes for the current tile.
    float fMinDepth = asfloat(group_uMinDepth);
    float fMaxDepth = asfloat(group_uMaxDepth);

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
					AddLightToTransparentList(i);
					AddLightToOpaqueList(i);
				}
				break;
				case POINT_LIGHT:
				{
					float3 lightPositionVS = mul(c_view, light.position).xyz;
					Sphere sphere = { lightPositionVS, light.range };
					if (SphereInsideFrustum(sphere, group_GroupFrustum, nearClipVS, maxDepthVS))
					{
						// Add light to light list for transparent geometry.
						AddLightToTransparentList(i);

						if (!SphereInsidePlane(sphere, minPlane))
						{
							// Add light to light list for opaque geometry.
							AddLightToOpaqueList(i);
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
		// InterlockedAdd guarantees that the group - shared light count variable is only updated by a single thread at a time.
		// This way we avoid any race conditions that may occur when multiple threads try to increment the group-shared light count at the same time.

		// Update light grid for opaque geometry.
		InterlockedAdd(rw_opaq_lightIndexCounter[0], group_opaq_LightCount, group_opaq_LightIndexStartOffset); // Performs a guaranteed atomic add of value to the dest resource variable.
		rw_opaq_lightGrid[input.groupID.xy] = uint2(group_opaq_LightIndexStartOffset, group_opaq_LightCount);

		// Update light grid for transparent geometry.
		InterlockedAdd(rw_trans_lightIndexCounter[0], group_trans_LightCount, group_trans_LightIndexStartOffset);
		rw_trans_lightGrid[input.groupID.xy] = uint2(group_trans_LightIndexStartOffset, group_trans_LightCount);
	}

	GroupMemoryBarrierWithGroupSync();

	// Update the light index list (all threads).
	// For opaque geometry.
	for (uint i = input.groupIndex; i < group_opaq_LightCount; i += TILE_SIZE * TILE_SIZE)
	{
		rw_opaq_lightIndexList[group_opaq_LightIndexStartOffset + i] = group_opaq_LightList[i];
	}

	// For transparent geometry.
	for (uint i = input.groupIndex; i < group_trans_LightCount; i += TILE_SIZE * TILE_SIZE)
	{
		rw_trans_lightIndexList[group_trans_LightIndexStartOffset + i] = group_trans_LightList[i];
	}


	// Update the debug texture output.
	if (input.groupThreadID.x == 0 || input.groupThreadID.y == 0)
	{
		rw_heatMap[texCoord] = float4(0, 0, 0, 1.0f);
	}
	else if (input.groupThreadID.x == 1 || input.groupThreadID.y == 1)
	{
		rw_heatMap[texCoord] = float4(1, 1, 1, 1.0f);
	}
	else if (group_opaq_LightCount > 0)
	{
		float normalizedLightCount = group_opaq_LightCount / 10.0f;
		float4 lightCountHeatMapColor = t_lightCountHeatMap.SampleLevel(s_linearClamp, float2(normalizedLightCount, 0), 0);
		rw_heatMap[texCoord] = lightCountHeatMapColor;
	}
	else
	{
		rw_heatMap[texCoord] = float4(0, 0, 0, 1);
	}
}