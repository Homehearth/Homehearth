#include "Phong.hlsli"

Texture2D T_albedo    : register(t0);
Texture2D T_normal    : register(t1);
Texture2D T_metalness : register(t2);
Texture2D T_roughness : register(t3);
Texture2D T_aomap     : register(t4);

SamplerState samp : register(s0);


/*
    Material constant buffers
*/
cbuffer matConstants_t : register(b0)
{
    float3 c_ambient;
    float  c_shiniess;
    float3 c_diffuse;
    float  c_opacity;
    float3 c_specular;
};
cbuffer properties_t : register(b1)
{
    //If a texture is set this will be 1
    int c_hasAlbedo;
    int c_hasNormal;
    int c_hasMetalness;
    int c_hasRoughness;
    int c_hasAoMap;
    int c_hasDisplace;
};

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
    if (c_hasAlbedo)
        return T_albedo.Sample(samp, input.uv);
    else
        return float4(1.0f, 0.0f, 0.0f, 1.0f);
}