#include "Common.hlsli"

struct PSIn
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

float4 main(PSIn input) : SV_TARGET
{
	float4 samp = t_depth.Sample(s_linear, input.uv);


	return samp;
}