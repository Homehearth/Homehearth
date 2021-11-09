cbuffer Matrices : register(b0)
{
    float4x4 c_world;       //row major
}

cbuffer Camera : register(b1)
{
    float4 c_cameraPosition;
    float4 c_cameraTarget;
    
    float4x4 c_projection;  //row major
    float4x4 c_view;        //row major
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
    float4 worldPos : WORLDPOSITION;
};

VertexOut main(VertexIn input)
{
    VertexOut output;
    
    output.pos = float4(input.pos, 1.0f);
    output.pos = mul(c_world, output.pos);
    output.worldPos = output.pos;
    output.pos = mul(c_view, output.pos);
    output.pos = mul(c_projection, output.pos);

    output.normal = mul((float3x3) c_world, input.normal);
    output.uv = input.uv;
    output.tangent = input.tangent;
    output.biTangent = input.biTangent;
    
    return output;
}