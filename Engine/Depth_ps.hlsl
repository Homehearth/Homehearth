struct VertexIn
{
    float4 pos : SV_POSITION;
};

struct VertexOut
{
    float depth : SV_DEPTH;
};

VertexOut main(VertexIn input)
{
    VertexOut output;
    normalize(input.pos);
	output.depth = input.pos.z / input.pos.w;
    return output;
}