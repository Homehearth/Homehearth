#include "Common.hlsli"

#define particle          particlesUAV[particleID.x]
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
    
    if (particle.life <= lifeTime)
    {
        particle.velocity.y += gravity * dt;

        float particleUniqueSpeed = speed * abs(randomNumbers[id + counter]);

        particle.velocity.x = cos(counter + id);
        particle.velocity.z = sin(counter + id);

        particle.pos += particle.velocity * speed * dt;

        if (particle.size.x > 0)
            particle.size -= ((particle.life) / (lifeTime)) * speed * dt;
    }
    else
    {
        particle.velocity = 0;
        particle.pos.y = emitterPos.y;
        particle.size = float2(sizeMulitplier, sizeMulitplier);
        particle.life = 0;

    }
}