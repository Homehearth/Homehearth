
//Texture2D diffuseTexture : register(t0);
SamplerState samp : register(s0);

struct PixelIn
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BINORMAL;
};

float4 main(PixelIn input) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
    //return diffuseTexture.Sample(samp, input.uv);
}