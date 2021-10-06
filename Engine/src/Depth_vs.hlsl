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

// Depth pre-pass VertexShader.
float4 main(VertexIn input) : SV_POSITION
{
    const float4 worldPosition = mul(world, float4(input.pos, 1.0f));
    return  mul(projectionView, worldPosition);
}
