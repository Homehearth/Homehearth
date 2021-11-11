#include "Common.hlsli"

//
// ForwardPlus: https://www.3dgep.com/forward-plus/
//

// Group shared variables.
groupshared uint group_uMinDepth;
groupshared uint group_uMaxDepth;
groupshared Frustum group_GroupFrustum;

groupshared uint group_opaq_LightCount;
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

	uint uDepth = asuint(fDepth);

	const float near = 0.01f;  
	const float far = 500.0f;

	uDepth = ((2.0f * near) / (far + near - uDepth * (far - near))); // Do I really need to ??

    if (input.groupIndex == 0) // Avoid contention by other threads in the group.
    {
        group_uMinDepth = 0xffffffff;
		group_uMaxDepth = 0;
		group_opaq_LightCount = 0;
		group_trans_LightCount = 0;
		group_GroupFrustum = sb_Frustums_in[input.groupID.x + (input.groupID.y * numThreadGroups.x)];
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
    const Plane minPlane = { float3(0, 0, -1), -minDepthVS };

    // Cull lights.
    // Each thread in a group will cull 1 light until all lights have been culled.
	for (uint i = input.groupIndex; i < MAX_LIGHTS; i += TILE_SIZE * TILE_SIZE)
	{
		if (sb_Lights[i].enabled)
		{
			const Light light = sb_Lights[i];

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
					const float3 lightPositionVS = mul(light.position, c_view).xyz;
					const Sphere sphere = { lightPositionVS, light.range };
					if (SphereInsideFrustum(sphere, group_GroupFrustum, nearClipVS, maxDepthVS))
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
		InterlockedAdd(rw_opaq_LightIndexCounter[0], group_opaq_LightCount, group_opaq_LightIndexStartOffset); // Performs a guaranteed atomic add of value to the dest resource variable.
		rw_opaq_LightGrid[input.groupID.xy] = uint2(group_opaq_LightIndexStartOffset, group_opaq_LightCount);

		// Update light grid for transparent geometry.
		InterlockedAdd(rw_opaq_LightIndexCounter[0], group_trans_LightCount, group_trans_LightIndexStartOffset);
		rw_trans_LightGrid[input.groupID.xy] = uint2(group_trans_LightIndexStartOffset, group_trans_LightCount);
	}

	GroupMemoryBarrierWithGroupSync();

	// Update the light index list (all threads).
	// For opaque geometry.
	for (uint i = input.groupIndex; i < group_opaq_LightCount; i += TILE_SIZE * TILE_SIZE)
	{
		rw_opaq_LightIndexList[group_opaq_LightIndexStartOffset + i] = group_opaq_LightList[i];
	}

	// For transparent geometry.
	for (uint i = input.groupIndex; i < group_trans_LightCount; i += TILE_SIZE * TILE_SIZE)
	{
		rw_trans_LightIndexList[group_trans_LightIndexStartOffset + i] = group_trans_LightList[i];
	}
}