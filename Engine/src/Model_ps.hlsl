#include "Common.hlsli"

float4 main(PixelIn input) : SV_TARGET
{
    static unsigned int rolls = infoData.x;
    
    
    /*
        This part of the code calculates if a decal should be present at this location.
        if-statement is to make sure the decal only gets placed on the world scene since its below 1.0f and everything else if either 1.0f or above.
    */
	if (input.worldPos.y < 1.0f)
    {
        [loop]
		for (int j = 0; j < rolls; j++)
		{
			float4 decal_pos = mul(sb_decaldata[j], input.worldPos);
			decal_pos = mul(decal_projection, decal_pos);

            // This is the same as shadow-mapping but instead of having different maps we have different view points.
			float2 texCoords;
			texCoords.x = decal_pos.x / decal_pos.w / 2.0f + 0.5f;
			texCoords.y = -decal_pos.y / decal_pos.w / 2.0f + 0.5f;
        
			if ((saturate(texCoords.x) == texCoords.x) & (saturate(texCoords.y) == texCoords.y))
			{
				float4 color = t_decal.Sample(s_linear, texCoords);

				return color;
			}
		}
	}
    
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
        
    }
    
	
    //Ambient lighting
    float3 ambient = float3(0.7f, 0.15f, 0.5f) * albedo * ao;
    float3 color = ambient + Lo;
    
    //HDR tonemapping
	color = color / (color + float3(1.0, 1.0, 1.0));
    //Gamma correct
    color = pow(max(color, 0.0f), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
	return float4(color, 0.0);    
}