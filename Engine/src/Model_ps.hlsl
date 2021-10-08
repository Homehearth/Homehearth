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

cbuffer Camera : register(b1)
{
    float4 cameraPosition;
    float4 cameraTarget;
    
    float4x4 projection;
    float4x4 view;
}

float4 main(PixelIn input) : SV_TARGET
{
    float3 camPos = cameraPosition.xyz;
    float ao = 1.0f;
    float3 albedo = 1.f;
    float metallic = 0.5f;
    float roughness = 0.5f;
    
    //Normal Vector
    float3 N = normalize(input.normal);
    //View Direction Vector
    float3 V = normalize(camPos - input.worldPos.xyz);
    
    //TEMP
    Light L[2];
    L[0].position = float4(0.f, 8.f, 10.f, 1.f);
    L[0].color = 10.f;
    L[0].direction = float4(-1.f, 0.f, 0.f, 0.f);
    L[0].range = 75.f;
    L[0].type = 0;
    L[0].enabled = 1;
    
    L[1].position = float4(0.f, 8.f, 10.f, 1.f);
    L[1].color = 300.f;
    L[1].direction = float4(0.f, -1.f, -1.f, 0.f);
    L[1].range = 75.f;
    L[1].type = 1;
    L[1].enabled = 1;
    
    
    //If an object has a texture sample from it, else use default values.
    if(c_hasAlbedo == 1)
    {
        albedo = pow(max(T_albedo.Sample(samp, input.uv).rgb, 0.0f), 2.2f); //Power the albedo by 2.2f to get it to linear space.
        
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
    float3 F0 = 0.04f;
    F0 = lerp(F0, albedo, metallic);

    //Reflectance Equation
    float3 Lo = 0.f;
    float3 rad = 0.f;
    float3 lightCol = 0.f;
    
    for (int i = 0; i < NR_LIGHTS; i++)
    {
        if(L[i].enabled = 1)
        {
            switch (L[i].type)
            {
                case 0:
                    lightCol += DoDirectionlight(L[i], N);
                    break;
                case 1:
                    lightCol += DoPointlight(L[i], input, N);
                    break;
                default:
                    break;
            }
        
            CalcRadiance(input, V, N, roughness, metallic, albedo, L[i].position.xyz, lightCol, F0, rad);
            Lo += rad;
        }
    }
    
	
    //Ambient lighting
    float3 ambient = float3(0.03f, 0.03f, 0.03f) * albedo * ao;
    float3 color = ambient + Lo;
    
    //HDR tonemapping
	color = color / (color + float3(1.0, 1.0, 1.0));
    //Gamma correct
    color = pow(max(color, 0.0f), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
	return float4(color, 0.0);    
}