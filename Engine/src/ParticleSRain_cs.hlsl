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
    float4 emitterToParticle = float4(0.0f, particle.pos.y, 0.0f, 1.f) - float4(0.0f, emitterPos.y, 0.0f, 1.f);
    if (length(emitterToParticle.y) < lifeTime)
    {
        particle.velocity.y += abs(randomNumbers[id + 1]) * dt;
        particle.velocity.y -= gravity * dt;
        particle.pos += particle.velocity * speed * dt;
    }
    else
    {
        particle.pos.y = emitterPos.y + (id / 2) * dt;
        particle.velocity.y = 0.0f;
        particle.life = 0;
    }
}