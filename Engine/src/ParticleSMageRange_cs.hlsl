#include "Common.hlsli"

#define particle        particlesUAV[particleID.x]
#define lifeTime        c_pLifeTime
#define counter         c_pCounter
#define speed           c_pSpeed
#define sizeMulitplier  c_pSizeMulitplier
#define emitterPos      c_pEmitterPosition
#define gravity         9.82f
#define dt              c_pDeltatime
#define dir             c_pDirection

[numthreads(50, 1, 1)]
void main(uint3 particleID : SV_DispatchThreadID)
{
    uint id = particleID.x;
    if (id >= 100)
        id -= 100;
    
    if (id > 25)
    {
        float particleLifeTime = (lifeTime - (randomNumbers[id + counter]));
        if (particle.life < particleLifeTime)
        {
            particle.velocity.y -= gravity * dt;
            particle.pos += particle.velocity * speed * dt;

            if (particle.size.x >= 0)
            {
                float sizeChange = abs((randomNumbers[id + counter]) * dt);
                particle.size -= sizeChange * sizeMulitplier * 4.f;
            }
        }
        else
        {
            particle.size = float2(1.5, 1.5);
            particle.pos = float4(emitterPos.x + (randomNumbers[id + counter] / 3), emitterPos.y + (randomNumbers[id] / 3), emitterPos.z + (randomNumbers[id + counter] / 3), emitterPos.w);
            particle.life = 0;

            particle.velocity.x = (randomNumbers[id]);
            particle.velocity.y = (randomNumbers[id + 1]);
            particle.velocity.z = (randomNumbers[id + counter]);
        }
    }
    else
    {
        particle.size = float2(3, 3);
        particle.pos = float4(emitterPos.x + (randomNumbers[id + counter] / 4), emitterPos.y + (randomNumbers[id] / 4), emitterPos.z + (randomNumbers[id + counter] / 4), emitterPos.w);
    }
}