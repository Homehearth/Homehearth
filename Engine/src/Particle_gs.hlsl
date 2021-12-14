#include "Common.hlsli"

[maxvertexcount(6)]
void main(point VertexParticleOut input[1], inout TriangleStream<PixelParticleIn> triStream)
{
    float2 cTextCoords[6] =
    {
        float2(1.0f, 1.0f),
        float2(0.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 0.0f)
    };
	
	 //Billboarding
    float3 upVector = float3(0.0f, 1.0f, 0.0f);
    float3 camToVert = input[0].pos.xyz - c_cameraPosition.xyz;
    float3 rightVector = normalize(cross(upVector, camToVert));
    
	//Compute trianglestrip vertesis into worldspace
    float halfWidth = 0.5f * input[0].size.x;
    float halfHeight = 0.5f * input[0].size.y;

    float4 right = float4(halfWidth * rightVector, 0.0f);
    float4 up = float4(halfHeight * upVector, 0.0f);

    
    float4 v[6];
    v[0] = input[0].pos + right - up; //World pos or float4()
    v[1] = input[0].pos - right - up;
    v[2] = input[0].pos - right + up;
    v[3] = input[0].pos + right + up;
    v[4] = v[0];
    v[5] = v[2];

    PixelParticleIn output;
	[unroll]
    for (int i = 0; i < 6; i++)
    {
        output.worldPos = v[i];
        output.pos = mul(c_view, v[i]);
        output.pos = mul(c_projection, output.pos);
        output.uv = cTextCoords[i];
        output.color = input[0].color;
        output.normal = normalize(camToVert);
        triStream.Append(output);
    }
}