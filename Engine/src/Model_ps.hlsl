#include "Phong.hlsli"
#include "PBR.hlsli"

Texture2D T_albedo    : register(t0);
Texture2D T_normal    : register(t1);
Texture2D T_metalness : register(t2);
Texture2D T_roughness : register(t3);
Texture2D T_aomap     : register(t4);
Texture2D T_displace  : register(t5);

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
cbuffer properties_t : register(b2)
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
    float4 worldPos : WORLDPOSITION;
};

cbuffer Camera : register(b1)
{
    float4 cameraPosition;
    float4 cameraTarget;
    
    float4x4 projection;
    float4x4 view;
}

//Calculates the outgoing radiance level of each light
void CalcRadiance(PixelIn input, float3 V, float3 N, float roughness, float metallic, float3 albedo, float3 lightPos, float3 lightCol, float3 F0, out float3 rad);

float4 main(PixelIn input) : SV_TARGET
{
    float3 camPos = cameraPosition.xyz;
    float ao = 1.0f;
    float3 albedo = float3(1.0f, 1.0f, 1.0f);
    float metallic = 0.5f;
    float roughness = 0.5f;
    
    float3 lightPos = float3(0.0f, 8.0f, -10.0f); //TODO: Light-struct to GPU
    float3 lightCol = float3(300.0f, 300.0f, 300.0f); //TODO: Light-struct to GPU   
    
    //Normal Vector
    float3 N = normalize(input.normal);
    //View Direction Vector
    float3 V = normalize(camPos - input.worldPos.xyz);
    
    //If an object has a texture sample from it, else use default values.
    if(c_hasAlbedo == 1)
    {
        albedo = T_albedo.Sample(samp, input.uv);
    }
    
    if(c_hasNormal == 1)
    {
        float3 normalMap = T_normal.Sample(samp, input.uv).rgb;
        normalMap = normalMap * 2.0f - 1.0f;
        
        float3 tangent = normalize(input.tangent.xyz);
        float3 biTangent = normalize(input.biTangent);
        float3x3 TBN = float3x3(tangent, biTangent, input.normal);
        
        N = normalize(mul(normalMap, TBN));
    }
    
    if(c_hasMetalness == 1)
    {
        metallic = T_metalness.Sample(samp, input.uv).r;
    }
    
    if(c_hasRoughness == 1)
    {
        roughness = T_roughness.Sample(samp, input.uv).r;
    }
    
    if(c_hasAoMap == 1)
    {
        ao = T_aomap.Sample(samp, input.uv).r;        
    }    
    

    //---------------------------------PBR-Shading Calculations---------------------------------
    
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
	float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);

    //Reflectance Equation
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    float3 rad = float3(0.0f, 0.0f, 0.0f);
    
    CalcRadiance(input, V, N, roughness, metallic, albedo, lightPos, lightCol, F0, rad);
    Lo += rad;
	
    //Ambient lighting
    float3 ambient = float3(0.03f, 0.03f, 0.03f) * albedo * ao;
    float3 color = ambient + Lo;
    
    //HDR tonemapping
	color = color / (color + float3(1.0, 1.0, 1.0));
    //Gamma correct
	color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
	return float4(color, 0.0);    
}


void CalcRadiance(PixelIn input, float3 V, float3 N, float roughness, float metallic, float3 albedo, float3 lightPos, float3 lightCol, float3 F0, out float3 rad)
{
    static const float PI = 3.14159265359;
    
    //Calculate Light Radiance
    float3 lightDir = normalize(lightPos - input.worldPos.xyz);
    float3 H = normalize(V + lightDir);
    float distance = length(lightPos - input.worldPos.xyz);
    float attenuation = 1.0f / dot(float3(1.0f, 0.0f, 1.0f), float3(1.0f, distance, distance * distance));
    float3 radiance = lightCol * attenuation;
    
    //Cook-Torrance BRDF
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, lightDir, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);
    
    float3 kS = F;
    float kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= (1.0 - metallic);
    
    float3 nom = D * G * F;
    float denom = 4 * max(dot(N, V), 0.0f) * max(dot(N, lightDir), 0.0f) + 0.001f;
    float3 specular = nom / denom;
    
    float NdotL = max(dot(N, lightDir), 0.0f);
    rad = (((kD * albedo / PI) + specular) * radiance * NdotL);

}