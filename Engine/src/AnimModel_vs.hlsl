#include "Common.hlsli"

VertexOut main(VertexBoneIn input)
{
    VertexOut output;
    
    float4x4 world;
    for (int i = 0; i < 4; i++)
    {
        world += sb_BoneTransforms[input.boneIDs[i]] * input.boneWeights[i];
    }

    //Global world-matrix has to affect the model
    world = mul(c_world, world);

    //Positions and worldpos
    output.pos      = float4(input.pos, 1.0f);
    output.pos      = mul(world, output.pos);
    output.worldPos = output.pos;
    output.pos      = mul(c_view, output.pos);
    output.pos      = mul(c_projection, output.pos);
    
    output.normal = mul((float3x3) world, input.normal);
    
    //Same as before
    output.uv           = input.uv;
    output.tangent      = input.tangent;
    output.biTangent    = input.biTangent;
    
    return output;
}