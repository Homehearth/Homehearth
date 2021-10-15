Texture2D diffuseTexture : register(t13);
Texture2D dissplacementTexture : register(t14);
SamplerState pointSampler : register(s1);

cbuffer textureEffectBuffer : register(b3)
{
    float deltaTime;
    float direction;
    float radious;
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    
    
    
}
