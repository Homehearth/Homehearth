#include "Common.hlsli"

#define vertex particlesUAV[particleID.x]

void BloodSimmulation(inout VertexParticleIn particle);
void LeafSimmulation(inout VertexParticleIn particle);
void WaterSplashSimmulation(inout VertexParticleIn particle);
void SmokeSimmulation(inout VertexParticleIn particle);
void SparklesSimmulation(inout VertexParticleIn particle);
void RainSimmulation(inout VertexParticleIn particle);

[numthreads(1, 1, 1)]
void main(uint3 particleID : SV_DispatchThreadID)
{
            
    if (vertex.type == 0)
        BloodSimmulation(vertex);
    
    else if (vertex.type == 1)
        LeafSimmulation(vertex);
    
    else if (vertex.type == 2)
        WaterSplashSimmulation(vertex);
    
    else if (vertex.type == 3)
        SmokeSimmulation(vertex);
    
    else if (vertex.type == 4)
        SparklesSimmulation(vertex);
    
    else if (vertex.type == 5)
        RainSimmulation(vertex);

}

void BloodSimmulation(inout VertexParticleIn particle)
{
	
}

void LeafSimmulation(inout VertexParticleIn particle)
{
}

void WaterSplashSimmulation(inout VertexParticleIn particle)
{
}

void SmokeSimmulation(inout VertexParticleIn particle)
{
    float raise = 0.05;
    
    particle.pos.y += raise;
    particle.size += 0.01;
    
    if (particle.pos.y >= 50) //Reset
    {
        particle.pos = particle.startPos;
        particle.size = (1, 1);

    }

}

void SparklesSimmulation(inout VertexParticleIn particle)
{
    
    particle.size = (2, 2);
    particle.color = (0, 1, 0, 1);

}

void RainSimmulation(inout VertexParticleIn particle)
{
    float raise = 0.05;
    
    particle.pos.y -= raise;
    
    if (particle.pos.y == -10) //Reset
    {
        particle.pos = particle.startPos;

    }
    
}
