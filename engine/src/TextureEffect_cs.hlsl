cbuffer textureEffectBuffer : register(b0)
{
    float deltaTime;
    float direction;
    float radious;
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}