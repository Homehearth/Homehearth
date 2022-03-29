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

[numthreads(50, 1, 1)]
void main(uint3 particleID : SV_DispatchThreadID)
{
    float4 emitterToParticle = particle.pos - emitterPos;
    if (length(emitterToParticle) < lifeTime) //LifeTime is here radius of heal circle
    {
        particle.pos += particle.velocity * speed * dt;
    }
    else
    {
        if (particle.size.x > 0)
            particle.size -= dt * sizeMulitplier * 2;

        if (particle.color.a > 0)
            particle.color.a -= dt * sizeMulitplier * 2;
    }
}