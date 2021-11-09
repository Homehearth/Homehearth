#include "PBR.hlsli"
cbuffer isColliding : register(b3)
{
    int colliding;
}

float4 main(PixelIn input) : SV_TARGET
{   
    if (colliding == 1)
        return float4(0.7f, 0.0f, 0.0f, 1.0f);
    else
        return float4(0.7f, 0.7f, 0.0f, 1.0f);
}