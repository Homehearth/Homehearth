#include "Common.hlsli"

float4 main(PixelIn input) : SV_TARGET
{
    float3 camPos = c_cameraPosition.xyz;
    float ao = 1.0f;
    float3 albedo = 1.f;
    float metallic = 1.0f;
    float roughness = 0.0f;
    
    //Normal Vector
    float3 N = normalize(input.normal);
    //View Direction Vector
    float3 V = normalize(camPos - input.worldPos.xyz);
    //Reflection Vector
    float3 R = reflect(-V, N);
    
    //If an object has a texture, sample from it else use default values.
    SampleTextures(input, albedo, N, roughness, metallic, ao);
    

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
        if(sb_lights[i].enabled == 1)
        {
            switch (sb_lights[i].type)
            {
                case 0:
                    lightCol += DoDirectionlight(sb_lights[i], N);
                    break;
                case 1:
                    lightCol += DoPointlight(sb_lights[i], input, N);
                    break;
                default:
                    break;
            }
        
            CalcRadiance(input, V, N, roughness, metallic, albedo, sb_lights[i].position.xyz, lightCol, F0, rad);
            Lo += rad;
        }
        lightCol = 0.f;
    }
    
	
    //Ambient lighting
    //float3 ambient = float3(0.7f, 0.15f, 0.5f) * albedo * ao;
    
    //IBL Ambient
    float3 kS = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metallic;
    
    float3 irradiance = t_cubeMap.Sample(s_linear, N).rgb;
    float3 diffuse = irradiance * albedo;
    
    const float MAX_REF_LOD = 4.0f;
    float3 prefilteredColor = t_cubeMap.SampleLevel(s_linear, R, roughness * MAX_REF_LOD).rgb;
    float2 brdf = t_BRDFLUT.Sample(s_linear, float2(max(dot(N, V), 0.0f), roughness)).rg;
    float3 specular = prefilteredColor * (kS * brdf.x + brdf.y);
    
    float3 ambient = (kD * diffuse + specular) * ao;
    float3 color = ambient + Lo;
    
    //HDR tonemapping
	color = color / (color + float3(1.0, 1.0, 1.0));
    //Gamma correct
    color = pow(max(color, 0.0f), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
	return float4(color, 0.0);
}