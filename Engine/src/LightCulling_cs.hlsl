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
groupshared Frustum group_GroupFrustum;

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
	const int2 texCoord = input.dispatchThreadID.xy;
	const float fDepth = t_depth.Load(int3(texCoord, 0)).r;
	const uint uDepth = asuint(fDepth);

    if (input.groupIndex == 0)
    {
        group_uMinDepth = 0xffffffff;
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

    float fMinDepth = asfloat(group_uMinDepth);
    float fMaxDepth = asfloat(group_uMaxDepth);

    // Convert depth values to view space.
    const float minDepthVS = ScreenToView(float4(0, 0, fMinDepth, 1)).z;
    const float maxDepthVS = ScreenToView(float4(0, 0, fMaxDepth, 1)).z;	// Used for spot light.
    const float nearClipVS = ScreenToView(float4(0, 0, 0, 1)).z;			// Used for spot light.

    // Clipping plane for minimum depth value 
    // (used for testing lights within the bounds of opaque geometry).
    const Plane minPlane = { float3(0, 0, 1), minDepthVS };

    // Cull lights.
    // Each thread in a group will cull 1 light until all lights have been culled.
	for (uint i = input.groupIndex; i < c_info.x; i += TILE_SIZE * TILE_SIZE)
	{
		if (sb_lights[i].enabled)
		{
			const Light light = sb_lights[i];

			switch (light.type)
			{
				case DIRECTIONAL_LIGHT:
				{
					AddLightToOpaqueList(i);
					AddLightToTransparentList(i);
				}
				break;
				case POINT_LIGHT:
				{
					const float4 lightPositionVS = mul(c_view, light.position);
					const Sphere sphere = { lightPositionVS.xyz, light.range };
					if (SphereInsideFrustum(sphere, group_GroupFrustum, nearClipVS, maxDepthVS)) // Z wrong ?
					{
						// Add light to light list for transparent geometry.
						AddLightToOpaqueList(i);

						if (!SphereInsidePlane(sphere, minPlane))
						{
							// Add light to light list for opaque geometry.
							AddLightToTransparentList(i);
						}
					}
				}
				break;
				default:
#pragma message( "You want spot lights? Implement it yourself then!")
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
		const float normalizedLightCount = group_opaq_LightCount / 50.0f;
		const float4 lightCountHeatMapColor = t_lightCountHeatMap.SampleLevel(s_linearClamp, float2(normalizedLightCount, 0), 0);

		// Wtf is wrong? Weird Texture2D.

		rw_heatMap[texCoord] = lightCountHeatMapColor;
	}
	else
	{
		rw_heatMap[texCoord] = float4(0, 0, 0, 1);
	}
}