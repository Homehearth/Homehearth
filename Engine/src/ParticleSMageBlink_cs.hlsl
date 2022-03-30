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
        float particleUniqueSpeed = speed * abs(randomNumbers[id + counter]);

        particle.velocity *= 1.0002f;
        particle.velocity = normalize(particle.velocity);
        particle.pos += particle.velocity * particleUniqueSpeed * dt;

        if (particle.size.x > 0)
            particle.size -= ((particle.life) / (lifeTime)) * particleUniqueSpeed * dt;

        if (particle.color.a > 0)
            particle.color.a -= dt * sizeMulitplier * 0.5f;

        particle.color.b += dt * sizeMulitplier * 10.f;
    }
    else
    {
        particle.pos = emitterPos;
        particle.life = 0;
    }
}