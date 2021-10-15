#include "PBR.hlsli"



StructuredBuffer<Light> S_Lights : register(t7);

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
    float4 c_cameraPosition;
    float4 c_cameraTarget;
    
    float4x4 c_projection;
    float4x4 c_view;
}

cbuffer LightsInfo : register(b3)
{
    float4 c_info = float4(0.f, 0.f, 0.f, 0.f);
}

float4 main(PixelIn input) : SV_TARGET
{
    float3 camPos = c_cameraPosition.xyz;
    float ao = 1.0f;
    float3 albedo = 1.f;
    float metallic = 0.5f;
    float roughness = 0.5f;
    
    //Normal Vector
    float3 N = normalize(input.normal);
    //View Direction Vector
    float3 V = normalize(camPos - input.worldPos.xyz);    
    
    //If an object has a texture, sample from it else use default values.
    float3 ANM = float3(c_hasAlbedo, c_hasNormal, c_hasMetalness);
    float RAD = float3(c_hasRoughness, c_hasAoMap, c_hasDisplace);
    SampleTextures(input, ANM, RAD, albedo, N, roughness, metallic, ao);
    

    //---------------------------------PBR-Shading Calculations---------------------------------
    
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    float3 F0 = 0.04f;
    F0 = lerp(F0, albedo, metallic);

    //Reflectance Equation
    float3 Lo = 0.f;
    float3 rad = 0.f;
    float3 lightCol = 0.f;
    
    for (int i = 0; i < c_info.x; i++)
    {
        if(S_Lights[i].enabled == 1)
        {
            switch (S_Lights[i].type)
            {
                case 0:
                    lightCol += DoDirectionlight(S_Lights[i], N);
                    break;
                case 1:
                    lightCol += DoPointlight(S_Lights[i], input, N);
                    break;
                default:
                    break;
            }
        
            CalcRadiance(input, V, N, roughness, metallic, albedo, S_Lights[i].position.xyz, lightCol, F0, rad);
            Lo += rad;
        }
        
    }
    
	
    //Ambient lighting
    float3 ambient = float3(0.4f, 0.4f, 0.4f) * albedo * ao;
    float3 color = ambient + Lo;
    
    //HDR tonemapping
	color = color / (color + float3(1.0, 1.0, 1.0));
    //Gamma correct
    color = pow(max(color, 0.0f), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
	return float4(color, 0.0);    
}