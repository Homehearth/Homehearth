
Texture2D T_albedo    : register(t0);
Texture2D T_normal    : register(t1);
Texture2D T_metalness : register(t2);
Texture2D T_roughness : register(t3);
Texture2D T_aomap     : register(t4);

SamplerState samp : register(s0);


//cbuffer textures_t
//{
//    bool hasAlbedo;
//    bool hasNormal;
//    bool hasMetalness;
//    bool hasRoughness;
//    bool hasAoMap;
//};

struct PixelIn
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BINORMAL;
};

float4 main(PixelIn input) : SV_TARGET
{
    //return float4(1.0f, 0.0f, 0.0f, 1.0f);
    return T_albedo.Sample(samp, input.uv);
}