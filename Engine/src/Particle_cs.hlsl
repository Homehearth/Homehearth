#include "Common.hlsli"

#define vertex particlesUAV[particleID.x]

void BloodSimmulation(inout VertexParticleIn particle);
void LeafSimmulation(inout VertexParticleIn particle);
void WaterSplashSimmulation(inout VertexParticleIn particle);
void SmokeSimmulation(inout VertexParticleIn particle, in int id, in uint3 partcileID);
void SparklesSimmulation(inout VertexParticleIn particle);
void RainSimmulation(inout VertexParticleIn particle);

[numthreads(1, 1, 1)]
void main(uint3 particleID : SV_DispatchThreadID)
{
    int id = particleID;
    if (id >= 100)
        id -= 100;
            
    if (vertex.type == 0)
        BloodSimmulation(vertex);
    else if (vertex.type == 1)
        LeafSimmulation(vertex);
    else if (vertex.type == 2)
        WaterSplashSimmulation(vertex);
    else if (vertex.type == 3)
        SmokeSimmulation(vertex, id, particleID);
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

void SmokeSimmulation(inout VertexParticleIn particle, in int id, in uint3 partcileID)
{  
    if (particle.pos.y < emitterPosition.y + 50 + randomNumbers[id])
    {
        particle.pos.x -= (3 + abs(randomNumbers[id + counter])) * deltaTime;
        particle.pos.z -= (3 + abs(randomNumbers[id + counter / 2])) * deltaTime;
                    

        
        if (partcileID.x % 2 == 0)
        {
            particle.pos.y += (2 + abs(randomNumbers[id + counter] + counter)) * deltaTime;
            particle.pos.x -= (1 + abs(randomNumbers[id + counter])) * deltaTime;
            particle.pos.z += (1 + abs(randomNumbers[id / 2])) * deltaTime;
        }
        else
        {
            particle.pos.y += (3 + abs(randomNumbers[id] + counter)) * deltaTime;
            particle.pos.x += (1 + abs(randomNumbers[id] + counter)) * deltaTime;
            particle.pos.z -= (1 + abs(randomNumbers[id + counter / 2])) * deltaTime;
        }
     
        particle.size += 1 * deltaTime;
                    
        if (particle.color.x > 20.f)
            particle.color = 10.f;
    }
    else
    {
        particle.size = (1,1);
        particle.pos = emitterPosition;
    }
}

void SparklesSimmulation(inout VertexParticleIn particle)
{
    
    particle.size = (2, 2);
    particle.color = (0.5f, 1, 0, 1);

}

void RainSimmulation(inout VertexParticleIn particle)
{
    float raise = 5  * deltaTime;
    
    particle.pos.y -= raise;
    
    if (particle.pos.y <= emitterPosition.y -20) //Reset
    {
        particle.pos = emitterPosition;

    }
    
}
