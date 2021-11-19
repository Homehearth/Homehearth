#include "Common.hlsli"

//
// ForwardPlus: https://www.3dgep.com/forward-plus/
//

#define LEFT            0
#define RIGHT           1
#define TOP             2
#define BOTTOM          3
#define NUM_CORNERS     4

#define TOP_LEFT        0
#define TOP_RIGHT       1
#define BOTTOM_LEFT     2
#define BOTTOM_RIGHT    3

[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void main(ComputeShaderIn input)
{
    // View space eye position is always at the origin.
    const float3 eyePos = float3(0, 0, 0);

    // Compute the 4 corner points on the far clipping plane to use as the frustum vertices.
    float4 screenSpace[NUM_CORNERS];
    screenSpace[TOP_LEFT]       = float4(input.dispatchThreadID.xy * TILE_SIZE, 1.0f, 1.0f);
    screenSpace[TOP_RIGHT]      = float4(float2(input.dispatchThreadID.x + 1, input.dispatchThreadID.y) * TILE_SIZE, 1.0f, 1.0f);
    screenSpace[BOTTOM_LEFT]    = float4(float2(input.dispatchThreadID.x, input.dispatchThreadID.y + 1) * TILE_SIZE, 1.0f, 1.0f);
    screenSpace[BOTTOM_RIGHT]   = float4(float2(input.dispatchThreadID.x + 1, input.dispatchThreadID.y + 1) * TILE_SIZE, 1.0f, 1.0f);

    float3 viewSpace[NUM_CORNERS];
    for (int i = 0; i < NUM_CORNERS; i++)
    {
        viewSpace[i] = ScreenToView(screenSpace[i]).xyz;
    }

    // Now build the frustum planes from the view space points.
    Frustum frustum;
    frustum.planes[LEFT]    = ComputePlane(eyePos, viewSpace[BOTTOM_LEFT], viewSpace[TOP_LEFT]);
    frustum.planes[RIGHT]   = ComputePlane(eyePos, viewSpace[TOP_RIGHT], viewSpace[BOTTOM_RIGHT]);
    frustum.planes[TOP]     = ComputePlane(eyePos, viewSpace[TOP_LEFT], viewSpace[TOP_RIGHT]);
    frustum.planes[BOTTOM]  = ComputePlane(eyePos, viewSpace[BOTTOM_RIGHT], viewSpace[BOTTOM_LEFT]);

    // Store the computed frustum in global memory (if our thread ID is in bounds of the grid).
    if (input.dispatchThreadID.x < numThreads.x && input.dispatchThreadID.y < numThreads.y)
    {
        const uint index = input.dispatchThreadID.x + (input.dispatchThreadID.y * numThreads.x);
        rw_Frustums_out[index] = frustum;
    }
}