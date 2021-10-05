
#include "Phong.hlsli"

Texture2D<float4> DepthTexture		: register(t0);
StructuredBuffer<Light> SceneLights : register(t1);
RWTexture2D<float4> OutputTexture	: register(u0);
SamplerState clampLinearSample		: register(s0);

groupshared uint minDepthInt;
groupshared uint maxDepthInt;
groupshared uint visibleLightCount = 0;
groupshared uint visibleLightIndices[1024];


/* groupshared:
 * Mark a variable for thread-group-shared memory for compute shaders.
 * In D3D11 the maximum total size of all variables with the groupshared storage class is 32kb. 
 */
// ThreadGroupSizeX, ThreadGroupSizeY
[numthreads(1, 1, 1)]
void main(	uint3 groupId			: SV_GroupID,
			uint3 groupThreadId		: SV_GroupThreadID,
			uint groupIndex			: SV_GroupIndex,
			uint3 dispatchThreadId	: SV_DispatchThreadID)
{
	// Divide each tile in computeShader, and one tile is generally 16x16 or 32x32.
	// Calculate the maximum and minimum PosZ values of all pixels of each tile(generally camera space is better).

	// Calculate MaxZ and MinZ for each Tiled's camera space.
	float depth = DepthTexture[dispatchThreadId.xy].r;

	
	// Calculate frustum for each tile.

	// Calculate the number of point light sources that intersect each Tiled and record their index.
}