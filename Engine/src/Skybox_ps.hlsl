#include "common.hlsli"

float4 main(PixelInSB input) : SV_TARGET
{
	//SAMPLE FROM CUBEMAP
    return t_sky.Sample(s_cubeSamp, input.texCoord);
}