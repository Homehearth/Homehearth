#include "common.hlsli"

float4 main(PixelInSB input) : SV_TARGET
{
	//SAMPLE FROM CUBEMAP
    return t_cubeMap.Sample(s_cubeSamp, input.texCoord);
}