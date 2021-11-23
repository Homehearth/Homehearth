#include "Common.hlsli"

float4 main(PixelIn input) : SV_TARGET
{
    //return t_shadowMaps.Sample(s_linear, float3(input.uv, 0.0f));

    static unsigned int rolls = infoData.x;
    
	float3 camPos = c_cameraPosition.xyz;
    float ao = 1.0f;
    float3 albedo = 1.f;
    float metallic = 0.0f;
    float roughness = 0.0f;
    float exposure = 0.1f;
    const float gamma = 1.f / 2.2f;
    
    //Normal Vector
    float3 N = normalize(input.normal);
    //View Direction Vector
    float3 V = normalize(camPos - input.worldPos.xyz);
    
    //If an object has a texture, sample from it else use default values.
    SampleTextures(input, albedo, N, roughness, metallic, ao);

    //---------------------------------PBR-Shading Calculations---------------------------------
    
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
   

    //Reflectance Equation
    float3 Lo = float3(0.f, 0.f, 0.f);
    float3 rad = float3(0.f, 0.f, 0.f);
    float3 lightCol = float3(0.f, 0.f, 0.f);
    [loop]
    for (int i = 0; i < c_info.x; i++)
    {
        if(sb_lights[i].enabled == 1)
        {
			float shadowCoef = 0.0f;
			if (sb_lights[i].direction.y < 0)
			{
				float4x4 lightMat = sb_lights[i].lightMatrix;
            
				float4 pixelposLightSpace = mul(lightMat, input.worldPos);
				pixelposLightSpace.xy /= pixelposLightSpace.w;
            
				float2 texCoords;
				texCoords.x = pixelposLightSpace.x * 0.5f + 0.5f;
				texCoords.y = -pixelposLightSpace.y * 0.5f + 0.5f;
            
				if ((saturate(texCoords.x) == texCoords.x) & (saturate(texCoords.y) == texCoords.y))
				{
					float closestDepth = t_shadowMaps.Sample(s_linear, float3(texCoords.xy, i)).r;
					//closestDepth = saturate(closestDepth);
                    
                    float currentDepth = pixelposLightSpace.z / pixelposLightSpace.w;
					//currentDepth = saturate(currentDepth);
					currentDepth -= 0.001f;
                    
					if (currentDepth > closestDepth)
					{
						shadowCoef = 1.0f;
						return float4(currentDepth - closestDepth, 0, 0, 1.0f);
					}
				}
			}
            
            switch (sb_lights[i].type)
            {
                case 0:
					lightCol += DoDirectionlight(sb_lights[i], N) * (1.0f - shadowCoef);
                    break;
                case 1:
					lightCol += DoPointlight(sb_lights[i], input, N) * (1.0f - shadowCoef);
                    break;
                default:
                    break;
            }
        
            CalcRadiance(input, V, N, roughness, metallic, albedo, sb_lights[i].position.xyz, lightCol, F0, rad);
            Lo += rad;
        }
    }
    
    //Ambient lighting
    float3 ambient = float3(0.7f, 0.15f, 0.5f) * albedo * ao;
    ambient = ambientIBL(albedo, N, V, F0, metallic, roughness, ao);
    
  
    /*
        This part of the code calculates if a decal should be present at this location.
        if-statement is to make sure the decal only gets placed on the world scene since its below 1.0f and everything else if either 1.0f or above.
    */

    if (input.worldPos.y < 1.0f)
    {
        [loop]
        for (unsigned int j = 0; j < rolls; j++)
        {
            float4 decal_pos = mul(sb_decaldata[j], input.worldPos);
            decal_pos = mul(decal_projection, decal_pos);

            // This is the same as shadow-mapping but instead of having different maps we have different view points.
            float2 texCoords;
            texCoords.x = decal_pos.x / decal_pos.w / 2.0f + 0.5f;
            texCoords.y = -decal_pos.y / decal_pos.w / 2.0f + 0.5f;
        
            if ((saturate(texCoords.x) == texCoords.x) & (saturate(texCoords.y) == texCoords.y))
            {
                float3 color = t_decal.Sample(s_linear, texCoords).xyz;
                float alpha = t_decal.Sample(s_linear, texCoords).r;
                
                if (alpha > 0.4f)
                {
                    color = (color * ambient) + Lo;
                    //HDR tonemapping
                    color = color / (color + float3(1.0, 1.0, 1.0));
                    //Gamma correct
                    color = pow(max(color, 0.0f), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
                    return float4(color, alpha);
                }

            }
        }
    }
    
    float3 color = ambient + Lo;
    
    //HDR tonemapping
	color = color / (color + float3(1.0, 1.0, 1.0));
    //Gamma correct
    color = pow(max(color, 0.0f), float3(gamma, gamma, gamma));
    
    return float4(color, 5.0f);
}