#include "Common.hlsli"

cbuffer decal_viewpoint : register(b10)
{
	float4   cameraPosition;
	float4   cameraTarget;
	float4x4 projection;  //row major
	float4x4 view;        //row major
}

struct VSIn
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VSOut
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VSOut main(VSIn input)
{
	VSOut output;

	output.position = mul(c_world, float4(input.position, 1.0f));
	output.position = mul(view, output.position);
	output.position = mul(c_projection, output.position);

	output.uv = input.uv;

	return output;
}