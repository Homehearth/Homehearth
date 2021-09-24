cbuffer Matrices : register(b0)
{
    float4x4 world;
}

cbuffer Camera : register(b1)
{
    float4 cameraPosition;
    float4 cameraTarget;
    
    float4x4 projection;
    float4x4 view;
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
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BINORMAL;
};

VertexOut main(VertexIn input)
{
    VertexOut output;
    
    output.pos = float4(input.pos, 1.0f);
    output.pos = mul(world, output.pos);
    output.pos = mul(view, output.pos);
    output.pos = mul(projection, output.pos);

    output.normal = mul(input.normal, (float3x3) world);
    output.uv = input.uv;
    output.tangent = input.tangent;
    output.biTangent = input.biTangent;
    return output;
}