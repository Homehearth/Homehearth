#include "Common.hlsli"

VertexParticleOut main(uint id : SV_InstanceID)
{
    VertexParticleOut output;
    
    output.pos = particlesInput[id].pos;
    output.size = particlesInput[id].size;
    output.color = particlesInput[id].color;
    
    return output;
}