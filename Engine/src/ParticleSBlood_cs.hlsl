#include "Common.hlsli"

#define vertex          particlesUAV[particleID.x]
#define lifeTime        c_pLifeTime
#define counter         c_pCounter
#define speed           c_pSpeed
#define sizeMulitplier  c_pSizeMulitplier
#define emitterPos      c_pEmitterPosition
#define gravity         9.82f
#define dt              c_pDeltatime
#define dir             c_pDirection

void BloodSimmulation(inout VertexParticleIn particle, in uint id);

[numthreads(50, 1, 1)]
void main(uint3 particleID : SV_DispatchThreadID)
{   
    uint id = particleID.x;
    if (id >= 100)
        id -= 100;
    
    BloodSimmulation(vertex, id);
    
}

void BloodSimmulation(inout VertexParticleIn particle, in uint id)
{
    float particleLifeTime = (lifeTime) - (randomNumbers[id + counter] / 2 * dt);
    
    if (particle.life < particleLifeTime)
    {
        particle.pos += (particle.velocity * 10.f) * speed * dt;
        
        particle.velocity.y -= gravity * dt;
        
        if (particle.size.x >= 0 && particle.life <= particleLifeTime / 3)
        {
            float sizeChange = abs((randomNumbers[id + counter]) * dt);
            particle.size -= sizeChange * ((lifeTime + sizeMulitplier) * 0.7f);
            particle.color.rgb += 1.0f * dt;
        }
        if (particle.size.x >= 0) 
            particle.size -= abs(randomNumbers[id + counter]) / 10.f;
    }
    else
    {
        particle.size = float2(sizeMulitplier, sizeMulitplier);
        particle.pos = emitterPos;
        particle.life = 0.f;

        particle.color = float4(0.8f, 0.8f, 0.8f, 1.f);
        
        particle.velocity.x = (randomNumbers[id]);
        particle.velocity.y = (randomNumbers[id + 1]);
        particle.velocity.z = (randomNumbers[id + counter]);
    }
}