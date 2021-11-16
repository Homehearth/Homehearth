#include "Common.hlsli"

#define vertex particlesUAV[particleID.x]

void BloodSimmulation(VertexParticleIn particle);
void LeafSimmulation(VertexParticleIn particle);
void WaterSplashSimmulation(VertexParticleIn particle);
void SmokeSimmulation(VertexParticleIn particle);
void SparklesSimmulation(VertexParticleIn particle);
void RainSimmulation(VertexParticleIn particle);

[numthreads(1024, 1, 1)]
void main(uint3 particleID : SV_DispatchThreadID)
{
    vertex.pos += 1;
    
    
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

void BloodSimmulation(VertexParticleIn particle)
{
	
}

void LeafSimmulation(VertexParticleIn particle)
{
}

void WaterSplashSimmulation(VertexParticleIn particle)
{
}

void SmokeSimmulation(VertexParticleIn particle)
{
    float raise = 1;
    
    particle.pos += raise;
    
   // if (particle.pos.y >= 200) 
     //   particle.pos = particle.startPos;

}

void SparklesSimmulation(VertexParticleIn particle)
{
}

void RainSimmulation(VertexParticleIn particle)
{
}