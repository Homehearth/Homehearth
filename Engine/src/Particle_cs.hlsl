#include "Common.hlsli"

#define RUN_SWITCHCASE 0

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
void LeafSimmulation(inout VertexParticleIn particle, in uint id);
void WaterSplashSimmulation(inout VertexParticleIn particle, in uint id);
void SmokePointSimmulation(inout VertexParticleIn particle, in uint id);
void SmokeAreaSimmulation(inout VertexParticleIn particle, in uint id);
void SparklesSimmulation(inout VertexParticleIn particle, in uint id);
void RainSimmulation(inout VertexParticleIn particle, in uint id);
void MageHealSimulation(inout VertexParticleIn particle, in uint id);
void MageRangeSimulation(inout VertexParticleIn particle, in uint id);
void ExplosionSimulation(inout VertexParticleIn particle, in uint id);
void MageBlinkSimulation(inout VertexParticleIn particle, in uint id);
void UpgradeSimulation(inout VertexParticleIn particle, in uint id);

#if RUN_SWITCHCASE
[numthreads(50, 1, 1)]
void main(uint3 particleID : SV_DispatchThreadID)
{
    uint id = particleID.x;
    if (id >= 100)
        id -= 100;
        
    [branch]
    switch (vertex.type)
    {
        case 0:
            BloodSimmulation(vertex, id);
            break;
        case 1:
            LeafSimmulation(vertex, id);
            break;
        case 2:
            WaterSplashSimmulation(vertex, id);
            break;
        case 3:
            SmokePointSimmulation(vertex, id);
            break;
        case 4:
            SmokeAreaSimmulation(vertex, id);
            break;
        case 5:
            SparklesSimmulation(vertex, id);
            break;
        case 6:
            RainSimmulation(vertex, id);
            break;
        case 8:
            MageHealSimulation(vertex, id);
            break;
        case 9:
            MageRangeSimulation(vertex, id);
            break;
        case 10:
            ExplosionSimulation(vertex, id);
            break;
        case 11:
            MageBlinkSimulation(vertex, id);
            break;
        case 12:
            UpgradeSimulation(vertex, id);
            break;
        default:
            break;
    }
    
    vertex.life += dt;
}
#else
[numthreads(50, 1, 1)]
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
        SmokePointSimmulation(vertex, id);
    else if (vertex.type == 4)
        SmokeAreaSimmulation(vertex, id);
    else if (vertex.type == 5)
        SparklesSimmulation(vertex, id);
    else if (vertex.type == 6)
        RainSimmulation(vertex, id);
    else if (vertex.type == 8)
        MageHealSimulation(vertex, id);
    else if (vertex.type == 9)
        MageRangeSimulation(vertex, id);
    else if (vertex.type == 10)
        ExplosionSimulation(vertex, id);
    else if (vertex.type == 11)
        MageBlinkSimulation(vertex, id);
    else if (vertex.type == 12)
        UpgradeSimulation(vertex, id);
    
    vertex.life += dt;
}
#endif

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

void LeafSimmulation(inout VertexParticleIn particle, in uint id) //OLD
{
    float particleLifeTime = (lifeTime - (randomNumbers[id + counter]));
    
    if (particle.life < particleLifeTime)
    {
        //particle.pos.y -= 1 * dt;
        //particle.pos.x += cos(counter);
        
        
        particle.pos.x += (randomNumbers[id] / 2) * dt;
        particle.pos.y += (randomNumbers[id + 1]) * dt;
        particle.pos.z -= (randomNumbers[id + counter] ) * dt;
    }
    else
    {
        particle.size = float2(1, 1);
        particle.pos = emitterPos;
        particle.life = counter / 2;
    }
}

void WaterSplashSimmulation(inout VertexParticleIn particle, in uint id) //OLD
{
    float particleLifeTime = (lifeTime - (randomNumbers[id + counter]));
    
    if (particle.life < particleLifeTime)
    {
        particle.pos.x += (2.0f * randomNumbers[id]) * dt;
        //particle.pos.y += ;//(2.0f * randomNumbers[id + 1]) * dt;
        particle.pos.z += (4.0f * abs(randomNumbers[id + counter])) * dt;
        
        particle.pos.y -= gravity * dt;

        particle.pos += particle.velocity * dt;
        
        particle.velocity.y -= 9.82f * dt;
        particle.color += dt;
        
        //if (particle.size.x >= 0)
        //    particle.size -= ((particle.life ) / (particleLifeTime )) * particleSizeMulitplier;

    }
    else
    {
        particle.size = float2(1.5, 1.5);
        particle.pos = float4(emitterPos.x + (randomNumbers[id + counter]), emitterPos.y + (randomNumbers[id]), emitterPos.z, emitterPos.w);
       //particle.pos = emitterPos; //+ randomNumbers[counter];
        particle.velocity = float4(0, 0, 0, 0);
        particle.life = 0;
        particle.color = float4(0, 0, 0.5, 0.5);
    }
}

void SmokePointSimmulation(inout VertexParticleIn particle, in uint id)
{
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
        particle.life = 0;
        particle.color.a = 1;
        particle.velocity = float4(0, 0, 0, 0);
    }
}

void SmokeAreaSimmulation(inout VertexParticleIn particle, in uint id)
{
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

void SparklesSimmulation(inout VertexParticleIn particle, in uint id) //OLD
{
    float particleLifeTime = (lifeTime) - (randomNumbers[id + counter] / 2 * dt);
    
    if (particle.life < particleLifeTime)
    {
        particle.pos += (particle.velocity * 10.f) * speed * dt;
        
        //particle.velocity.y -= gravity * dt;
        
        if (particle.size.x >= 0 && particle.life <= particleLifeTime / 3)
        {
            float sizeChange = abs((randomNumbers[id + counter]) * dt);
            particle.size -= ((particle.life / 3) / (particleLifeTime / 3)) * sizeMulitplier;
            particle.color.rgb += 1.0f * dt;
        }
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

void RainSimmulation(inout VertexParticleIn particle, in uint id)
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
        particle.pos.y = emitterPos.y + (id/2 ) * dt;
        particle.velocity.y = 0.0f;
        particle.life = 0;
    }
}

void MageHealSimulation(inout VertexParticleIn particle, in uint id)
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

void MageRangeSimulation(inout VertexParticleIn particle, in uint id)
{    
    if (id > 25)
    {
        float particleLifeTime = (lifeTime - (randomNumbers[id + counter]));
        if (particle.life < particleLifeTime)
        {
            particle.velocity.y -= gravity * dt;
            particle.pos += particle.velocity * speed *dt;
                
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
        particle.size = float2(3,3);
        particle.pos = float4(emitterPos.x + (randomNumbers[id + counter] / 4), emitterPos.y + (randomNumbers[id] / 4), emitterPos.z + (randomNumbers[id + counter] / 4), emitterPos.w);
    }
}

void ExplosionSimulation(inout VertexParticleIn particle, in uint id)
{    
    particle.pos += particle.velocity * speed * dt;
    particle.size += dt * sizeMulitplier;
        
    if (particle.color.a > 0)
        particle.color.a -= dt * sizeMulitplier * 0.5f;
    
}

void MageBlinkSimulation(inout VertexParticleIn particle, in uint id)
{
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

void UpgradeSimulation(inout VertexParticleIn particle, in uint id)
{
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
