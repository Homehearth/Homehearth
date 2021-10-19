
struct ComputeShaderInput
{
    uint3 GroupID           : SV_GroupID;          
    uint3 GroupThreadID     : SV_GroupThreadID;    
    uint3 DispatchThreadID  : SV_DispatchThreadID; 
    uint  GroupIndex        : SV_GroupIndex;       
};

struct Plane
{
    float3 normal;
    float  dist_to_origin;
};

struct Frustum
{
    Plane planes[4];   // left, right, top, bottom frustum planes.
};

RWStructuredBuffer<Frustum> RWFrustums : register(u0);  

cbuffer Camera : register(b1)
{
    float4 cameraPosition;
    float4 cameraTarget;

    float4x4 projection;
    float4x4 view;
}


cbuffer DispatchParams : register(b2)
{
    uint3 NumThreadGroups;
    uint3 NumThreads;
};

#define BLOCK_SIZE 16

// Compute a plane from 3 noncollinear points that form a triangle.
Plane ComputePlane(float3 p0, float3 p1, float3 p2)
{
    Plane plane;

    float3 v0 = p1 - p0;
    float3 v2 = p2 - p0;

    plane.normal = normalize(cross(v0, v2));

    // Compute the distance to the origin using p0.
    plane.dist_to_origin = dot(plane.normal, p0);

    return plane;
}

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(ComputeShaderInput input )
{
    // View space eye position is always at the origin.
    const float3 eyePos = float3(0, 0, 0);

    // Compute 4 points on the far clipping plane to use as the 
    // frustum vertices.
    float4 screenSpace[4];
    // Top left point
    screenSpace[0] = float4(input.DispatchThreadID.xy * BLOCK_SIZE, 1.0f, 1.0f);
    // Top right point
    screenSpace[1] = float4(float2(input.DispatchThreadID.x + 1, input.DispatchThreadID.y) * BLOCK_SIZE, 1.0f, 1.0f);
    // Bottom left point
    screenSpace[2] = float4(float2(input.DispatchThreadID.x, input.DispatchThreadID.y + 1) * BLOCK_SIZE, 1.0f, 1.0f);
    // Bottom right point
    screenSpace[3] = float4(float2(input.DispatchThreadID.x + 1, input.DispatchThreadID.y + 1) * BLOCK_SIZE, 1.0f, 1.0f);

    float3 viewSpace[4];
	
    // Convert the screen space points to view space
    for (int i = 0; i < 4; i++)
    {
        // Convert to normalized texture coordinates in the range [0..1].
        //float2 texCoord = screenSpace[i].xy / screenDimensions;

        // Convert to clip space
        //float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screenSpace[i].z, screenSpace[i].w);
 	
        // Convert clip space coordinates to view space
        
        // View space position.
       // float4 view = mul(inverseProjection, clip);

        // Perspecitive projection.
        //view = view / view.w;
           	
        //viewSpace[i] = view.xyz;
    }

    // Now build the frustum planes from the view space points
    Frustum frustum;

    frustum.planes[0] = ComputePlane(eyePos, viewSpace[2], viewSpace[0]); // Left plane
    frustum.planes[1] = ComputePlane(eyePos, viewSpace[1], viewSpace[3]); // Right plane
    frustum.planes[2] = ComputePlane(eyePos, viewSpace[0], viewSpace[1]); // Top plane
    frustum.planes[3] = ComputePlane(eyePos, viewSpace[3], viewSpace[2]); // Bottom plane
	
	// Store the computed frustum in global memory (if our thread ID is in bounds of the grid).
    //if (input.DispatchThreadID.x < DispatchParams.NumThreads.x && input.DispatchThreadID.y < DispatchParams.NumThreads.y)
    {
      //  uint index = input.DispatchThreadID.x + (input.DispatchThreadID.y * DispatchParams.NumThreads.x);
       // RWFrustums[index] = frustum;
    }
}