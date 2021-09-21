cbuffer Matrices : register(b0)
{
    float4x4 world;
}

cbuffer Camera : register(b1)
{
    float4x4 view;
    float4x4 projection;
    float4 position;
    float4 lookAt;
}

struct VertexIn
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BINORMAL;
};

struct VertexOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BINORMAL;
};

VertexOut main(VertexIn input)
{
    VertexOut output;
    output.pos = mul(mul(mul(float4(input.pos, 1.0f), world), view), projection);
    output.normal = mul(input.normal, (float3x3) world);
    output.uv = input.uv;
    output.tangent = input.tangent;
    output.biTangent = input.biTangent;
    return output;
}