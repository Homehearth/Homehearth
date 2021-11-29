#include "Common.hlsli"

float4 main(PixelParticleIn input) : SV_TARGET
{
	
    float3 finalColor;
    
    finalColor = t_albedo.Sample(s_linear, input.uv).rgb;
    float opacity = t_opacitymask.Sample(s_linear, input.uv).r;
    
    opacity *= input.color.a;
    finalColor *= input.color.rgb;
    
    clip(t_albedo.Sample(s_linear, input.uv).a - 0.2f);
        
    return float4(finalColor, opacity);
}