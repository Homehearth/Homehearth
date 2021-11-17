#include "Common.hlsli"

#define vertex particlesUAV[particleID.x]

//void BloodSimmulation(VertexParticleIn particle);
//void LeafSimmulation(VertexParticleIn particle);
//void WaterSplashSimmulation(VertexParticleIn particle);
//void SmokeSimmulation(VertexParticleIn particle);
//void SparklesSimmulation(VertexParticleIn particle);
//void RainSimmulation(VertexParticleIn particle);

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
    float raise = 0.05;
    
    particle.pos.x += raise;
    particle.color = (0, 0, 1, 1);
    
   // if (particle.pos.y >= 200) 
     //   particle.pos = particle.startPos;

}

void SparklesSimmulation(VertexParticleIn particle)
{
    particle.size = (10, 10);
    particle.color = (0,1,0,1);

}

void RainSimmulation(VertexParticleIn particle)
{
}

[numthreads(1, 1, 1)]
void main(uint3 particleID : SV_DispatchThreadID)
{
    
    float raise = 0.01;
    
    vertex.pos.y += raise;
    
    if (vertex.type == 0)
        BloodSimmulation(vertex);
    
    else if (vertex.type == 1)
        LeafSimmulation(vertex);
    
    else if (vertex.type == 2)
        WaterSplashSimmulation(vertex);
    
    else if (vertex.type == 3)
        SmokeSimmulation(vertex);
    
    else if (vertex.type == 4)
    {
        SparklesSimmulation(particlesUAV[particleID.x]);
    }
    
    else if (vertex.type == 5)
        RainSimmulation(vertex);

}