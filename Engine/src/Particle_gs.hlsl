#include "Common.hlsli"

[maxvertexcount(3)]
void main(point VertexParticleIn input[1], inout TriangleStream<VertexParticleOut> triStream)
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
    float3 rightVector = -normalize(cross(camToVert, upVector));
    
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

    VertexParticleOut gOutput;
	[unroll]
    for (int i = 0; i < 4; i++)
    {
        gOutput.worldPos = mul(v[i], c_view);
        gOutput.worldPos = mul(gOutput.worldPos, c_projection);
        gOutput.uv = cTextCoords[i];
        gOutput.color = input[0].color;
        gOutput.normal = normalize(camToVert);
        triStream.Append(gOutput);
    }
}