cbuffer Matrices : register(b0) //Not needed later
{
    float4x4 c_world;
}

cbuffer Camera : register(b1)
{
    float4 c_cameraPosition;
    float4 c_cameraTarget;
    float4x4 c_projection;  //row major
    float4x4 c_view;        //row major
}

//cbuffer BonesInfo : register(b2)
//{
//    uint c_nrOfBones;
//};

StructuredBuffer<Matrix> s_boneTransforms : register(t11);

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
    
    //column major
    float4x4 world;
    for (int i = 0; i < 4; i++)
    {
        world += s_boneTransforms[input.boneIDs[i]] * input.boneWeights[i];
    }

    //world = mul(world, c_world);

    //Positions and worldpos
    output.pos = float4(input.pos, 1.0f);
    output.pos = mul(world, output.pos);
    output.worldPos = output.pos;
    output.pos = mul(c_view, output.pos);
    output.pos = mul(c_projection, output.pos);
    
    output.normal = mul((float3x3) world, input.normal);
    
    //Same as before
    output.uv           = input.uv;
    output.tangent      = input.tangent;
    output.biTangent    = input.biTangent;
    
    return output;
}