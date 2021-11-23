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
    
    float speed = vertex.pos.w; //+ (randomNumbers[id] / 10.0f);
    
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
    float particleLifeTime = (lifeTime + (randomNumbers[id + counter] * deltaTime));
    
    if (particle.life < particleLifeTime)
    {
                
        particle.velocity.x += (5.0f * randomNumbers[id]) * deltaTime;
        particle.velocity.y += (4.0f * randomNumbers[id + 1]) * deltaTime;
        particle.velocity.z += (5.0f * randomNumbers[id + counter]) * deltaTime;

        particle.pos += particle.velocity * deltaTime;
        
        particle.velocity.y -= 5.82f * deltaTime;
        
        //particle.pos.y -= 0.9f * deltaTime;
    
        //float lerpValue;
        //lerp(particle.pos.y, emitterPosition.y, lerpValue);
        //particle.pos.y = lerpValue;
        
        //particle.pos.x += (5.0f * randomNumbers[id]) * deltaTime;
        //particle.pos.y += (4.0f * randomNumbers[id + 1]) * deltaTime;
        //particle.pos.z += (5.0f * randomNumbers[id + counter]) * deltaTime;

        //particle.pos.y -= 0.9f * deltaTime;
        
        //if (particle.size.x >= 0)
        //    particle.size -= abs((randomNumbers[id]) * deltaTime);
    }
    else
    {
        particle.size = float2(1, 1);
        particle.pos = emitterPosition;
        particle.life = 0;
        particle.velocity = (0, 0, 0, 0);

    }
    

}

void LeafSimmulation(inout VertexParticleIn particle, in uint id)
{
    if (particle.life < (lifeTime + (randomNumbers[id + counter])))
    {
        //particle.pos.y -= 1 * deltaTime;
        //particle.pos.x += cos(counter);
        
        
        particle.pos.x += (randomNumbers[id] / 2) * deltaTime;
        particle.pos.y += (randomNumbers[id + 1]) * deltaTime;
        particle.pos.z -= (randomNumbers[id + counter] ) * deltaTime;
    }
    else
    {
        particle.size = float2(1, 1);
        particle.pos = emitterPosition;
        particle.life = counter / 2;
    }
}

void WaterSplashSimmulation(inout VertexParticleIn particle, in uint id)
{
    if (particle.life < (lifeTime + (randomNumbers[id + counter])))
    {
        particle.pos.x += (2.0f * randomNumbers[id]) * deltaTime;
        //particle.pos.y += ;//(2.0f * randomNumbers[id + 1]) * deltaTime;
        particle.pos.z += (4.0f * abs(randomNumbers[id + counter])) * deltaTime;
        
        particle.pos.y -= 9.82f * deltaTime;

        particle.pos += particle.velocity * deltaTime;
        
        particle.velocity.y -= 9.82f * deltaTime;
        
        if (particle.size.x >= 0)
            particle.size -= abs(randomNumbers[id] * deltaTime);
    }
    else
    {
        particle.size = float2(1.5, 1.5);
        //particle.pos = (emitterPosition.x + (randomNumbers[id + counter] / 10.f) , emitterPosition.y + (randomNumbers[id]/10.f), emitterPosition.z, emitterPosition.w);
        particle.pos = emitterPosition; //+ randomNumbers[counter];
        particle.velocity = (0, 0, 0, 0);
        particle.life = 0;
    }
}

void SmokeSimmulation(inout VertexParticleIn particle, in uint id)
{
    float particleLifeTime = (lifeTime + (randomNumbers[id + counter]));
    
    if (particle.life < particleLifeTime)
    {
        //particle.pos.y += (abs(randomNumbers[id])) * deltaTime;
        //particle.pos.z -= ((randomNumbers[id + counter])) * deltaTime;
        //particle.pos.x += ((randomNumbers[id / 2])) * deltaTime;
        
        particle.velocity.x += (1.0f * randomNumbers[id]) * deltaTime;
        particle.velocity.y += (1.0f * randomNumbers[id + 1]) * deltaTime;
        particle.velocity.z += (1.0f * randomNumbers[id + counter]) * deltaTime;

        particle.pos += particle.velocity * deltaTime;
        
        particle.velocity.y += 5.82f * deltaTime;
        
        
        particle.size += randomNumbers[id + counter] * deltaTime;
        
        if (particle.color.a >= 0 && particle.life >= particleLifeTime - 1.5)
            particle.color.a -= deltaTime;
        
        //particle.color.rgb += deltaTime;
        
    }
    else
    {
        particle.size = float2(1, 1);
        particle.pos = emitterPosition;
        particle.life = counter;
        particle.color.a = 1;
        particle.velocity = (0, 0, 0, 0);

    }
}

void SparklesSimmulation(inout VertexParticleIn particle, in uint id)
{
    
    if (particle.life < (lifeTime + (randomNumbers[id + counter])))
    {
        particle.pos.x += (2.0f * randomNumbers[id]) * deltaTime;
        particle.pos.y += (2.0f * randomNumbers[id + 1]) * deltaTime;
        particle.pos.z += (2.0f * randomNumbers[id + counter]) * deltaTime;
        
        if (particle.size.x >= 0)
            particle.size -= abs(randomNumbers[id] * deltaTime);
    }
    else
    {
        particle.size = float2(1.5, 1.5);
        particle.pos = emitterPosition;
        particle.life = 0;
    }

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


///particle.pos.x -= (3 + abs(randomNumbers[id + counter])) * deltaTime;
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