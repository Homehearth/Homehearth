#include "Common.hlsli"

float4 main(PixelIn input) : SV_TARGET
{
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
    
    // For testing Depth Buffer.
    //const int3 posCoords = int3(input.pos.xy, 0);
    //const float depth = t_depth.Load(posCoords).x;
    //return float4(depth, depth, depth, 1.0f);

    //---------------------------------PBR-Shading Calculations---------------------------------
    
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
   

    //Reflectance Equation
    float3 Lo = float3(0.f, 0.f, 0.f);
    float3 rad = float3(0.f, 0.f, 0.f);
    float3 lightCol = float3(0.f, 0.f, 0.f);

    // Get the index of the current pixel in the light grid.
	const uint2 tileIndex = uint2(floor(input.pos.xy / (TILE_SIZE)));

    // Get the start position and offset of the light in the light index list.
	const uint startOffset = LightGrid[tileIndex].x;
	const uint lightCount = LightGrid[tileIndex].y;
    
    //for (int i = 0; i < c_info.x; i++)
    for (int i = 0; i < lightCount; i++)
    {
        const uint lightIndex = LightIndexList[startOffset + i];

        if(sb_Lights[lightIndex].enabled == 1)
        {
            switch (sb_Lights[lightIndex].type)
            {
                case 0:
                    lightCol += DoDirectionlight(sb_Lights[lightIndex], N);
                    break;
                case 1:
                    lightCol += DoPointlight(sb_Lights[lightIndex], input, N);
                    break;
                default:
                    break;
            }
        
            CalcRadiance(input, V, N, roughness, metallic, albedo, sb_Lights[lightIndex].position.xyz, lightCol, F0, rad);
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
                float alpha = t_decalAlpha.Sample(s_linear, texCoords).r;
                
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