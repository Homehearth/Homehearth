#include "Common.hlsli"

VertexOut main(VertexIn input)
{
    VertexOut output;
    
    output.pos = float4(input.pos, 1.0f);
    output.pos = mul(c_world, output.pos);
    output.worldPos = output.pos;
    output.pos = mul(c_view, output.pos);
    output.pos = mul(c_projection, output.pos);

    output.normal       = mul((float3x3) c_world, input.normal);
    output.uv           = input.uv;
    output.tangent      = input.tangent;
    output.biTangent    = input.biTangent;
    
    return output;
}