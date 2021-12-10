#include "Common.hlsli"

void main(PixelIn input)
{
    float alpha = 1.f;
    if (c_hasOpacity)
    {
        alpha = t_opacitymask.Sample(s_linear, input.uv).r;
    }
    else if (c_hasAlbedo)
    {
        alpha = t_albedo.Sample(s_linear, input.uv).a;
    }

	clip(alpha < 0.9f ? -1 : 1); // discard.
}