#include "common.hlsli"

float4 main(PixelInSB input) : SV_TARGET
{
	//SAMPLE FROM CUBEMAP
    
    float3 color = pow(max(t_sky.Sample(s_cubeSamp, input.texCoord), 0.0f), 2.2f).rgb * c_tint;
    
    //HDR tonemapping
    color = color / (color + float3(1.0, 1.0, 1.0));
    //Gamma correct
    color = pow(max(color, 0.0f), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    return float4(color, 5.f);
}