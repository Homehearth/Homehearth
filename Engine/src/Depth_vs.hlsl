#include "Common.hlsli"

// Depth pre-pass.
VertexOut main(VertexIn input)
{
    VertexOut output;

	output.pos = float4(input.pos, 1.0f);
    output.pos = mul(c_world, output.pos);
    output.pos = mul(c_view, output.pos);
    output.pos = mul(c_projection, output.pos);
    output.uv = input.uv;
    return output;
}
