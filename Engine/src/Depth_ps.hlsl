#include "Common.hlsli"

void main(VertexOut input)
{
    float alpha = 1;
    if (c_hasOpacity)
    {
        alpha = t_opacitymask.Sample(s_linear, input.uv.xy).r;
    }
    else if (c_hasAlbedo)
    {
        alpha = t_albedo.Sample(s_linear, input.uv.xy).a;
    }

	clip(alpha < 0.5f ? -1 : 1); // discard.
}