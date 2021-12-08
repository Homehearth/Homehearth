#include "Common.hlsli"

struct PixelInBloom
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PixelInBloom input) : SV_TARGET
{
    //float x;
    //float y;
    //uint nr;
    //t_albedo.GetDimensions(0, x, y, nr);
    //float u_offset = (1.0f / x) / 2.0f;
    //float v_offset = (1.0f / y) / 2.0f;
    return t_albedo.Sample(s_linear, input.uv * 4.0f);
}