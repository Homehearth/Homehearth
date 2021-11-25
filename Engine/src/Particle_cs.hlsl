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
    //vertex.size += particleSizeMulitplier * deltaTime;

}

void BloodSimmulation(inout VertexParticleIn particle, in uint id)
{
    float particleLifeTime = (lifeTime + (randomNumbers[id + counter] * deltaTime));
    
    if (particle.life < particleLifeTime)
    {
                


        particle.pos += (particle.velocity * 10) * deltaTime;
        
        particle.velocity.y -= 4.82f * deltaTime;
        
        if (particle.size.x >= 0)
        {
            float sizeChange = abs((randomNumbers[id + counter]) * deltaTime);
            particle.size -= sizeChange * particleSizeMulitplier;
            particle.color.rgb += 1.0 * deltaTime;
        }


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
        particle.size = float2(2, 2);
        particle.pos = emitterPosition;
        particle.life = 0;

        particle.color = float4(0.8, 0.8 ,0.8 ,1 );
        
        particle.velocity.x = (randomNumbers[id]);
        particle.velocity.y = (randomNumbers[id + 1]) ;
        particle.velocity.z = (randomNumbers[id + counter]);
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
        particle.color += deltaTime;
        
        if (particle.size.x >= 0)
        {
            float sizeChange = abs((randomNumbers[id + counter]) * deltaTime);
            particle.size -= sizeChange * particleSizeMulitplier;
        }
    }
    else
    {
        particle.size = float2(1.5, 1.5);
        //particle.pos = (emitterPosition.x + (randomNumbers[id + counter] / 10.f) , emitterPosition.y + (randomNumbers[id]/10.f), emitterPosition.z, emitterPosition.w);
        particle.pos = emitterPosition; //+ randomNumbers[counter];
        particle.velocity = float4(0, 0, 0, 0);
        particle.life = 0;
        particle.color = float4(0, 0, 0.5, 0.5);
    }
}

void SmokeSimmulation(inout VertexParticleIn particle, in uint id)
{
    float particleLifeTime = (lifeTime + (randomNumbers[id + counter]));
    
    if (particle.life < particleLifeTime)
    {
        
        particle.velocity.x += (randomNumbers[id]) * deltaTime;
        particle.velocity.y += abs(randomNumbers[id + 1]) * deltaTime;
        particle.velocity.z += (randomNumbers[id + counter]) * deltaTime;

        particle.pos += particle.velocity * deltaTime;
        
        particle.velocity.y += 9.82 * deltaTime;
        
        float sizeChange = abs(randomNumbers[id + counter]) * deltaTime;
        particle.size += sizeChange * particleSizeMulitplier;
        
        if (particle.color.a >= 0 && particle.life >= particleLifeTime - 1.5)
        {
            particle.color.a -= 2.5 * deltaTime;
            //particle.color.rgb -= deltaTime;
        }
    }
    else
    {
        particle.size = float2(1, 1);
        particle.pos = emitterPosition;
        //particle.pos.x = emitterPosition.x + (randomNumbers[id] * 4);
        //particle.pos.z = emitterPosition.z + (randomNumbers[id + 1] *4);
        //particle.pos.y = emitterPosition.y + (randomNumbers[id] * lifeTime);
        particle.life = 0;
        particle.color.a = 1;
        particle.velocity = float4(0, 0, 0, 0);
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
        {
            float sizeChange = abs((randomNumbers[id + counter]) * deltaTime);
            particle.size -= sizeChange * particleSizeMulitplier;
        }
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