cbuffer Matrices : register(b0)
{
	float4x4 world;
}

cbuffer Camera : register(b1)
{
    float4x4 projectionView;
}

struct VertexIn
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BINORMAL;
};

// Depth pre-pass.
float4 main(VertexIn input) : SV_POSITION
{
    float4 position = float4(input.pos, 1.0f);
    position = mul(world, position);
    return mul(projectionView, position);
}
