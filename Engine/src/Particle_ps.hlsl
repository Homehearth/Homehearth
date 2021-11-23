#include "Common.hlsli"

//Texture2D t_albedo : register(t1);

float4 main(PixelParticleIn input) : SV_TARGET
{
	
    float3 finalColor;
    
    finalColor = t_albedo.Sample(s_linear, input.uv).rgb;
    float opacity = t_opacitymask.Sample(s_linear, input.uv).r;
    
    finalColor = finalColor * input.color.rgb;
    
        
    return float4(finalColor, (opacity));
}