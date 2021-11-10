#include "common.hlsli"

float4 main(PixelInSB input) : SV_TARGET
{
	//SAMPLE FROM CUBEMAP
    return t_cubeMap.SampleLevel(s_cubeSamp, input.texCoord, 0.f);
}