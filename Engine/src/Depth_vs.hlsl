#include "Common.hlsli"

// Depth pre-pass.
float4 main(VertexIn input) : SV_POSITION
{
    float4 position = float4(input.pos, 1.0f);
    position = mul(c_world, position);
    position = mul(c_view, position);
    return mul(c_projection, position);
}
