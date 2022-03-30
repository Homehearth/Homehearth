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
    
    float particleLifeTime = (lifeTime - (randomNumbers[id + counter]));
    if (particle.life < particleLifeTime)
    {
        particle.velocity.x += (randomNumbers[id]) * dt;
        particle.velocity.y += abs(randomNumbers[id + 1]) * dt;
        particle.velocity.z += (randomNumbers[id + counter]) * dt;

        particle.pos += particle.velocity * speed * dt;

        particle.velocity.y += gravity * dt;

        float sizeChange = abs(randomNumbers[id + counter]) * dt;
        particle.size += sizeChange * sizeMulitplier;

        if (particle.color.a >= 0 && particle.life >= particleLifeTime - 2.0)
        {
            particle.color.a -= dt;
            particle.color.rgb -= dt;
        }
    }
    else
    {
        particle.size = float2(1, 1);
        particle.pos = emitterPos;
        particle.pos.x = emitterPos.x + ((randomNumbers[id] * 4) / 2);
        particle.pos.y = emitterPos.y + ((randomNumbers[id] * lifeTime) / 2);
        particle.pos.z = emitterPos.z + ((randomNumbers[id + 1] * 4) / 2);
        particle.life = 0;
        particle.color.a = 1;
        particle.velocity = float4(0, 0, 0, 0);
    }
}