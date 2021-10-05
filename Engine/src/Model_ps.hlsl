#include "Phong.hlsli"
#include "PBR.hlsli"

//Texture2D albedoTexture : register(t0);
//Texture2D normalTexture : register(t1);
//Texture2D roughnessTexture : register(t2);
//Texture2D metallicTexture : register(t3);
//Texture2D aoTexture : register(t4);
SamplerState samp : register(s0);

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
    float3 albedo = float3(0.5f, 0.0f, 0.0f); // = albedoTexture.Sample(anisotropic, input.uv);
    float metallic = 0.0f; // = metallicTexture.Sample(anisotropic, input.uv).r;
    float roughness = 0.9f; // = roughnessTexture.Sample(anisotropic, input.uv).r;
    float ao = 1.0; // = aoTexture.Sample(anisotropic, input.uv).r;
    float3 lightPos = float3(0.0f, 8.0f, -17.0f); //TODO: Light-struct to GPU
    float3 lightCol = float3(1.0f, 1.0f, 1.0f); //TODO: Light-struct to GPU

    float3 N = normalize(input.normal);
    float3 V = normalize(camPos - input.worldPos.xyz);

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
    float3 ambient = float3(0.5f, 0.5f, 0.5f) * albedo * ao;
    
    float3 color = ambient + Lo;
    
    //HDR tonemapping
	//color = color / (color + float3(1.0, 1.0, 1.0));
    //Gamma correct
	//color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
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