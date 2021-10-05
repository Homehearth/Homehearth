#include "Phong.hlsli"

Texture2D DepthTexture : register(t1);
SamplerState LinearSampler : register(s0);

struct PixelIn
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BINORMAL;
};

float4 main(PixelIn input) : SV_TARGET
{
    const float surfaceDepth = DepthTexture.Sample(LinearSampler, input.uv.xy).r;
    return float4(surfaceDepth, surfaceDepth, surfaceDepth, 1.0f);
}