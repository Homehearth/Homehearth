#include "Common.hlsli"

void main(PixelIn input)
{
	float4 albedoSamp = t_albedo.Sample(s_linear, input.uv);
	if (albedoSamp.a < 0.5f)
		clip(-1);
}