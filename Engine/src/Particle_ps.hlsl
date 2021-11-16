#include "Common.hlsli"

//Texture2D t_albedo : register(t1);

float4 main(PixelParticleIn input) : SV_TARGET
{
	
    float4 finalColor;
    
    finalColor = t_albedo.Sample(s_linear, input.uv);
    finalColor = finalColor * input.color;
        
    return finalColor;
}