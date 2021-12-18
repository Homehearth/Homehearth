#include "Common.hlsli"

//
// ForwardPlus: https://www.3dgep.com/forward-plus/
//

#define TOP             0
#define RIGHT           1
#define BOTTOM          2
#define LEFT            3
#define NUM_CORNERS     4

#define TOP_LEFT        0
#define TOP_RIGHT       1
#define BOTTOM_RIGHT    2
#define BOTTOM_LEFT     3

// A kernel to compute frustums for the grid
// This kernel is executed once per grid cell. Each thread
// computes a frustum for a grid cell.
[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void main(ComputeShaderIn input)
{
    // View space eye position is always at the origin.
    const float3 eyePos = float3(0, 0, 0);
   
    // Compute the 4 corner points on the far clipping plane to use as the frustum vertices.
    // clockwise from top-left
    float4 screenSpace[NUM_CORNERS];
    screenSpace[TOP_LEFT]       = float4(input.dispatchThreadID.xy * TILE_SIZE, 1.0f, 1.0f);
    screenSpace[TOP_RIGHT]      = float4(float2(input.dispatchThreadID.x + 1, input.dispatchThreadID.y) * TILE_SIZE, 1.0f, 1.0f);   // assuming left-handed coord: z is positive.
    screenSpace[BOTTOM_RIGHT]   = float4(float2(input.dispatchThreadID.x + 1, input.dispatchThreadID.y + 1) * TILE_SIZE, 1.0f, 1.0f);
    screenSpace[BOTTOM_LEFT]    = float4(float2(input.dispatchThreadID.x, input.dispatchThreadID.y + 1) * TILE_SIZE, 1.0f, 1.0f);

    float3 viewSpace[NUM_CORNERS];
    for (int i = 0; i < NUM_CORNERS; i++)
    {
        viewSpace[i] = ScreenToView(screenSpace[i]).xyz;
    }

    // create plane equations for the four sides of the frustum, 
	// remember view space is left handed, so use the left-hand rule to determine 
	// cross product direction
    Frustum frustum;
    frustum.planes[TOP]     = ComputePlane(viewSpace[TOP_LEFT], viewSpace[TOP_RIGHT]);
    frustum.planes[RIGHT]   = ComputePlane(viewSpace[TOP_RIGHT], viewSpace[BOTTOM_RIGHT]);
    frustum.planes[BOTTOM]  = ComputePlane(viewSpace[BOTTOM_RIGHT], viewSpace[BOTTOM_LEFT]);
    frustum.planes[LEFT]    = ComputePlane(viewSpace[BOTTOM_LEFT], viewSpace[TOP_LEFT]);

    // Store the computed frustum in global memory (if our thread ID is in bounds of the grid).
    if (input.dispatchThreadID.x < numThreads.x && input.dispatchThreadID.y < numThreads.y) 
    {
        uint index = input.dispatchThreadID.x + (input.dispatchThreadID.y * numThreads.x);
        out_Frustums[index] = frustum;
    }
}