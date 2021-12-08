#include "Common.hlsli"

struct PixelInBloom
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PixelInBloom input) : SV_TARGET
{
    return t_albedo.Sample(s_linear, input.uv * c_info.x);
}