/*cbuffer Matrices : register(b0)
{
    float4x4 c_world;
}*/

cbuffer Camera : register(b1)
{
    float4 c_cameraPosition;
    float4 c_cameraTarget;
    
    float4x4 c_projection;
    float4x4 c_view;
}

StructuredBuffer<float4x4> s_boneTransforms : register(t11);

struct VertexIn
{
    float3 pos          : POSITION;
    float2 uv           : TEXCOORD;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float3 biTangent    : BINORMAL;
    uint4  boneIDs      : BONEIDS;
    float4 boneWeights  : BONEWEIGHTS;
};

struct VertexOut
{
    float4 pos          : SV_POSITION;
    float2 uv           : TEXCOORD;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float3 biTangent    : BINORMAL;
    float4 worldPos     : WORLDPOSITION;
};

VertexOut main(VertexIn input)
{
    VertexOut output;
    
    float4x4 world;
    for (int i = 0; i < 4; i++)
    {
        world += s_boneTransforms[input.boneIDs[i]] * input.boneWeights[i];
    }
    
    float4x4 wvp = mul(world, mul(c_view, c_projection));
    
    output.worldPos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(float4(input.pos, 1.0f), wvp);  
    output.normal = mul((float3x3) world, input.normal);
    
    //Same as before
    output.uv           = input.uv;
    output.tangent      = input.tangent;
    output.biTangent    = input.biTangent;
    
    return output;
}