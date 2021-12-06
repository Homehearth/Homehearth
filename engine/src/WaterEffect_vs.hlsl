#include "Common.hlsli"

VertexOut main(VertexIn input)
{
    VertexOut output;
    
    output.pos = float4(input.pos, 1.0f);
    output.normal = input.normal;
    output.uv = input.uv;
    output.tangent = input.tangent;
    output.biTangent = input.biTangent;
    
    return output;
}
