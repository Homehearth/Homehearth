#include "Common.hlsli"

PixelOut main(PixelIn input)
{
    //return t_shadowMaps.Sample(s_linear, float3(input.uv, 0.0f));
    PixelOut output;
    static unsigned int rolls = infoData.x;
    static float LIGHT_RANGE = 215.0f;
    const float LIGHT_VOLUME_RANGE = 250.0f;
    static unsigned int STEPS = c_info.y;
    const float SCATTERING = .85f;
    
    float3 lightVolume = float3(1.0f, 1.0f, 1.0f);
	float3 camPos = c_cameraPosition.xyz;
    float ao = 1.0f;
    float3 albedo = 1.f;
    float metallic = 0.0f;
    float roughness = 0.0f;
    const float gamma = 1.f / 2.2f;
    const float3 scatter = float3(SCATTERING, SCATTERING, SCATTERING);
    
    //Normal Vector
    float3 N = normalize(input.normal);
    //View Direction Vector
    float3 V = normalize(camPos - input.worldPos.xyz);
    
    //If an object has a texture, sample from it else use default values.
    SampleTextures(input, albedo, N, roughness, metallic, ao);
    //albedo = ACESFitted(albedo);
    
    
    //-----------------------------------------------------------------
    // WATER FOAM //
    /*
    The higher the level of vertex color, the more the foam should show.
    */
    
    float3 threshold = float3(0.99, 0.99, 0.99);
    float3 finalFoam = float3(0.f, 0.f, 0.f);
    
    float3 foam = t_waterBlend.Sample(s_linear, input.uv).xyz;
    foam = foam * 4;
    
    //Makes a nice fade
    threshold = threshold * input.color.r;
    finalFoam.rgb += ((input.color.r * input.color.r) * 0.5);
    threshold *= input.color.b;
    

    //Makes blobs of foam 
    if (foam.r >= threshold.r)
    {
        finalFoam = foam;
        finalFoam = finalFoam * input.color.b;
    }
    
    //Adds the effect
    finalFoam = finalFoam * input.color.b;
    
    //-----------------------------------------------------------------
    
    
    
    
    
    //If normal texture exists, sample from it

    //---------------------------------PBR-Shading Calculations---------------------------------
    
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo * 15.f, metallic);
   

    //Reflectance Equation
    float3 Lo = float3(0.f, 0.f, 0.f);
    float3 rad = float3(0.f, 0.f, 0.f);
    float3 lightCol = float3(0.f, 0.f, 0.f);
    [loop]
    for (int i = 0; i < c_info.x; i++)
    {
        if(sb_lights[i].enabled == 1)
        {
			float4x4 lightMat = sb_lights[i].lightMatrix;
			float shadowCoef = 0.0f;
            
            // position of this pixel in the light clip space
            float4 pixelposLightSpace = mul(lightMat, float4(input.worldPos.xyz, 1.0f));
			
            int shadowIndex = sb_lights[i].shadowIndex;
            
            uint width, height, elementCount;
            t_shadowMaps.GetDimensions(width, height, elementCount);
            int shadowMapSize = width;
            
            int blurKernalSize = 5;
            
            
            switch (sb_lights[i].type)
            {
                case 0:
                {
                    pixelposLightSpace.xy /= pixelposLightSpace.w;
            
                    float2 texCoords;
                    texCoords.x = pixelposLightSpace.x * 0.5f + 0.5f;
                    texCoords.y = -pixelposLightSpace.y * 0.5f + 0.5f;
            
                    if ((saturate(texCoords.x) == texCoords.x) & (saturate(texCoords.y) == texCoords.y))
			        {
                        //calculate current fragment depth
                        float currentDepth = pixelposLightSpace.z / pixelposLightSpace.w;
                        currentDepth = saturate(currentDepth);
                        currentDepth -= 0.001f;
                    
                        shadowCoef = SampleShadowMap(texCoords, shadowIndex, currentDepth, shadowMapSize, blurKernalSize);
                    }
                    
                    // Volumetric Lighting
                        if (length(camPos - input.worldPos.xyz) < LIGHT_VOLUME_RANGE)
                        {
                            float3 currentPos = camPos;
                            float3 rayVector = input.worldPos.xyz - camPos;
                            float3 rayDir = rayVector / length(rayVector);
                            float3 stepLength = length(rayVector) / STEPS;
                            float3 step = rayDir * stepLength;
                            [loop]
                            for (uint j = 0; j < STEPS; j++)
                            {
                        // Camera position in shadow space.
                                float4 cameraShadowSpace = mul(lightMat, float4(currentPos, 1.0f));
                                float2 shadowCoords;
                        
                        // Sample the depth of current position.
                                shadowCoords.x = cameraShadowSpace.x * 0.5f + 0.5f;
                                shadowCoords.y = -cameraShadowSpace.y * 0.5f + 0.5f;
                                float depth = t_shadowMaps.Sample(s_linear, float3(shadowCoords, shadowIndex)).r;
                                if (depth > cameraShadowSpace.z & ((saturate(shadowCoords.x) == shadowCoords.x) & (saturate(shadowCoords.y) == shadowCoords.y)))
                                {
                                    lightVolume += sb_lights[i].color.xyz / 255.0f;
                                }
                        
                                currentPos += step;
                            }
                            lightVolume /= STEPS;
                        }
                   
                    
                    lightCol += DoDirectionlight(sb_lights[i], N) * (1.0f - shadowCoef);
                    finalFoam *= (1.0f - shadowCoef);
                    finalFoam *= 0.6;
                    break;
                }
                case 1:
				{

                        float len = length(pixelposLightSpace.xyz);
                        pixelposLightSpace.xyz /= len;
                        float closestDepth = 1.0f;
                        float currentDepth = 0.0f;
                        float2 texCoords;
                     
                        if(pixelposLightSpace.z >= 0.0f)
                        {
                            //bottom Paraboloid
                            texCoords.x = (pixelposLightSpace.x / (1.0f + pixelposLightSpace.z)) * 0.5f + 0.5f;
                            texCoords.y = 1.0f - ((pixelposLightSpace.y / (1.0f + pixelposLightSpace.z)) * 0.5f + 0.5f);

                            //calculate current fragment depth
                            currentDepth = (len - 0.1f) / (500.f - 0.1f);
                            currentDepth -= 0.001f;
                        

                            shadowCoef = SampleShadowMap(texCoords, shadowIndex, currentDepth, shadowMapSize, blurKernalSize);
                        
                        }
                        else
                        {
                            //Top Paraboloid
                            texCoords.x = 1.0f - (pixelposLightSpace.x / (1.0f - pixelposLightSpace.z)) * 0.5f + 0.5f;
                            texCoords.y = 1.0f - ((pixelposLightSpace.y / (1.0f - pixelposLightSpace.z)) * 0.5f + 0.5f);
                            currentDepth = (len - 0.1f) / (500.f - 0.1f);
                            currentDepth -= 0.001f;
                        
                            shadowCoef = SampleShadowMap(texCoords, shadowIndex, currentDepth, shadowMapSize, blurKernalSize);

                        }
                    
                        lightCol += DoPointlight(sb_lights[i], input, N) * (1.0f - shadowCoef);
                        finalFoam += (DoPointlight(sb_lights[i], input, N) * (1.0f - shadowCoef) * 0.5);
                    
                    break;
                }
                default:
                    break;
            }
        
            CalcRadiance(input, V, N, roughness, metallic, albedo, sb_lights[i].position.xyz, lightCol, F0, rad);
            Lo += rad;
            finalFoam += rad;
            finalFoam *= input.color.b;
        }
    }
    
    //finalFoam *= rad;
    
    //Ambient lighting
    float3 staticAmbient = float3(0.7f, 0.15f, 0.5f) * albedo * ao;
    float3 ambient = ambientIBL(albedo, N, V, F0, metallic, roughness, ao);
    
    // FOG
    float3 pos = input.worldPos.xyz;
    pos.y = 0;
    float3 toCenter = float3(247, 0, -350) - pos;
    float distanceToCenter = length(toCenter);

    float4 fogColor = float4(0.04f, 0.06f, 0.2f, 1);
    //float4 fogColor = float4(0.5f, 0.5f, 0.5f, 1);

    float fogFactor = saturate((distanceToCenter - 150.f) / 100.f);
    float lightVolumeFactor = lightVolume.x > 0.0f ? lightVolume.x : 1.0f;
  
    /*
        This part of the code calculates if a decal should be present at this location.
        if-statement is to make sure the decal only gets placed on the world scene since its below 1.0f and everything else if either 1.0f or above.
    */

    if (input.worldPos.y < 1.0f)
    {
        [loop]
        for (unsigned int j = 0; j < rolls; j++)
        {
            float4 decal_pos = mul(sb_decaldata[j], float4(input.worldPos.xyz, 1.0f));
            decal_pos = mul(decal_projection, decal_pos);

            // This is the same as shadow-mapping but instead of having different maps we have different view points.
            float2 texCoords;
            texCoords.x = decal_pos.x / decal_pos.w / 2.0f + 0.5f;
            texCoords.y = -decal_pos.y / decal_pos.w / 2.0f + 0.5f;
        
            if ((saturate(texCoords.x) == texCoords.x) & (saturate(texCoords.y) == texCoords.y))
            {
                float3 albedoDecal  = t_decal.Sample(s_linear, texCoords).xyz;
                float alpha         = t_decal.Sample(s_linear, texCoords).r;
                
                if (alpha > 0.4f)
                {
                    float3 colorDecal = staticAmbient + (Lo * 0.2f); //ambientIBL(albedoDecal, float3(0, 1, 0), V, F0, 0.f, 0.2f, 1.f) + Lo;
                    
                    //HDR tonemapping
                    //colorDecal = colorDecal / (colorDecal + float3(1.0, 1.0, 1.0));
                    colorDecal = ACESFitted(colorDecal);
                    //Gamma correct
                    colorDecal = pow(max(colorDecal, 0.0f), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
                    colorDecal = lerp(colorDecal, fogColor.xyz, fogFactor);
                    output.color =  float4(colorDecal, alpha);
                    output.brightColor =  float4(0,0,0,0);
                    return output;
                }

            }
        }
    }
    
    
    
    float3 color = (ambient + Lo) * lightVolumeFactor;   
    float brightness = dot(color, float3(0.2126, 0.7152, 0.0722));
    
    color += finalFoam;
    
    //HDR tonemapping
    color = ACESFitted(color);
    //Gamma correct
    color = pow(max(color, 0.0f), float3(gamma, gamma, gamma));
    
    color = lerp(color, fogColor.xyz, fogFactor);
    
    
    //Bloom stuff
    if (brightness > 1.0f)
        output.brightColor = float4(color, 1.0f);
    else
        output.brightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    output.color = float4(color, 5.0f);
    return output;
}