#include "Common.hlsli"

float4 main(PixelParticleIn input) : SV_TARGET
{
    const float gamma = 1.f / 2.2f;
    
    float4 albedoSamp = t_albedo.Sample(s_linear, input.uv);
    clip(albedoSamp.a < 0.5f ? -1 : 1);
    float3 albedo = pow(max(albedoSamp.rgb, 0.f), 2.2f);    
    albedo *= input.color.rgb;
    
    float opacity = t_opacitymask.Sample(s_linear, input.uv).r;
    
    float3 N = normalize(input.normal);
    float3 V = normalize(c_cameraPosition.xyz - input.worldPos.xyz);
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    
    float3 ambient = ambientIBL(albedo, N, V, F0, 0.f, 1.0f, 1.f);
    
    float3 finalColor = albedo * c_tint;
    
    //opacity *= input.color.a;
    //finalColor *= input.color.rgb;      
    
    //finalColor = ACESFitted(finalColor);
    //finalColor = pow(max(finalColor, 0.0f), float3(gamma, gamma, gamma));
    
    return float4(finalColor, opacity);
}