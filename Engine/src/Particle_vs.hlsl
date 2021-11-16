#include "Common.hlsli"

VertexParticleOut main(uint id : SV_InstanceID)
{
    VertexParticleOut output;
    
    output.pos = particlesSRV[id].pos;
    output.size = particlesSRV[id].size;
    output.color = particlesSRV[id].color;
    
    return output;
}