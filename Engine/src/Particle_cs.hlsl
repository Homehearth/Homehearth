#include "Common.hlsli"

#define vertex particlesUAV[particleID.x]

void BloodSimmulation(inout VertexParticleIn particle, in uint id);
void LeafSimmulation(inout VertexParticleIn particle, in uint id);
void WaterSplashSimmulation(inout VertexParticleIn particle, in uint id);
void SmokeSimmulation(inout VertexParticleIn particle, in uint id);
void SparklesSimmulation(inout VertexParticleIn particle, in uint id);
void RainSimmulation(inout VertexParticleIn particle, in uint id);

[numthreads(1, 1, 1)]
void main(uint3 particleID : SV_DispatchThreadID)
{
    uint id = particleID.x;
    if (id >= 100)
        id -= 100;
            
    if (vertex.type == 0)
        BloodSimmulation(vertex, id);
    else if (vertex.type == 1)
        LeafSimmulation(vertex, id);
    else if (vertex.type == 2)
        WaterSplashSimmulation(vertex, id);
    else if (vertex.type == 3)
        SmokeSimmulation(vertex, id);
    else if (vertex.type == 4)
        SparklesSimmulation(vertex, id);
    else if (vertex.type == 5)
        RainSimmulation(vertex, id);

    vertex.life += deltaTime;
}

void BloodSimmulation(inout VertexParticleIn particle, in uint id)
{
    particle.pos.y += (abs(randomNumbers[id + counter] + counter)) * deltaTime;
    particle.pos.x += ((randomNumbers[id + counter])) * deltaTime;
    particle.pos.z += ((randomNumbers[id / 2])) * deltaTime;
     
    particle.size += randomNumbers[id + counter] * 2 * deltaTime;
                    
    if (particle.color.x > 20.f)
        particle.color = 10.f;
}

void LeafSimmulation(inout VertexParticleIn particle, in uint id)
{
}

void WaterSplashSimmulation(inout VertexParticleIn particle, in uint id)
{
}

void SmokeSimmulation(inout VertexParticleIn particle, in uint id)
{
    if (particle.life < (lifeTime + (randomNumbers[id + counter])))
    {
        //particle.pos.x -= (3 + abs(randomNumbers[id + counter])) * deltaTime;
        //particle.pos.z -= (3 + abs(randomNumbers[id + counter / 2])) * deltaTime;
        
        
        //particle.pos.x += particle.velocity * deltaTime;
        //particle.pos.y += (randomNumbers[id]) * deltaTime;
        //particle.pos.z += particle.velocity * deltaTime;


        
        //if (partcileID.x % 2 == 0)
        //{
        //    particle.pos.y += ( abs(randomNumbers[id + counter] + counter)) * deltaTime;
        //    particle.pos.x += ( (randomNumbers[id + counter])) * deltaTime;
        //    particle.pos.z += ( (randomNumbers[id / 2])) * deltaTime;
        //}
        //else
        //{
        //    particle.pos.y += ( abs(randomNumbers[id] + counter)) * deltaTime;
        //    particle.pos.x += ( (randomNumbers[id] + counter)) * deltaTime;
        //    particle.pos.z += ( (randomNumbers[id + counter / 2])) * deltaTime;
        //}
        
        particle.pos.y += (abs(randomNumbers[id + counter] + counter)) * deltaTime;
        particle.pos.x += ((randomNumbers[id + counter])) * deltaTime;
        particle.pos.z += ((randomNumbers[id / 2])) * deltaTime;
     
        particle.size += randomNumbers[id + counter] * 2 * deltaTime;
                    
        //if (particle.color.x > 20.f)
        //    particle.color = 10.f;
        
        
    }
    else
    {
        particle.size = float2(1, 1);
        particle.pos = emitterPosition;
        particle.life = counter / 2;
        particle.color = (0, 0, 0, 0);

    }
}

void SparklesSimmulation(inout VertexParticleIn particle, in uint id)
{
    
    particle.size = float2(2, 2);
    particle.color = float4(0.5f, 1, 0, 1);

}

void RainSimmulation(inout VertexParticleIn particle, in uint id)
{
    float raise = 5  * deltaTime;
    
    particle.pos.y -= raise;
    
    if (particle.pos.y <= emitterPosition.y -20) //Reset
    {
        particle.pos = emitterPosition;

    }
    
}
