#include "Common.hlsli"

VertexParticleOut main(VertexIn input)
{
    VertexParticleOut output;
    
    output.pos = float4(input.pos, 1.0f);
    output.pos = mul(c_world, output.pos);
    output.worldPos = output.pos;
    output.pos = mul(c_view, output.pos);
    output.pos = mul(c_projection, output.pos);

    
    return output;
}