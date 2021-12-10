#include "Common.hlsli"

float4 main(PixelParticleIn input) : SV_TARGET
{
    const float gamma = 1.f / 2.2f;
    
    float4 albedoSamp = t_albedo.Sample(s_linear, input.uv);
    clip(albedoSamp.a < 0.2f ? -1 : 1);
    float3 albedo = pow(max(albedoSamp.rgb, 0.f), 2.2f);    
    albedo *= input.color.rgb;
    
    float opacity = t_opacitymask.Sample(s_linear, input.uv).r;
    opacity *= input.color.a;
    
    float3 finalColor;
    if (c_pParticleType != 9 & c_pParticleType != 0 & c_pParticleType != 6 & c_pParticleType != 10)
    {
        finalColor = albedo * c_tint;
    }
    else
    {
        finalColor = albedo;
    }
    finalColor = albedo;

    finalColor = ACESFitted(finalColor);
    finalColor = pow(max(finalColor, 0.0f), float3(gamma, gamma, gamma));
    
    return float4(finalColor, opacity);
}