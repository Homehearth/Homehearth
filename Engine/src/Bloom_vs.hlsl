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
    
    output.position = float4(input.position, 1.0f);
    output.uv = input.uv;
	
    return output;
}