#ifndef _COMMON_HLSLI_
	#error You may not include this header directly.
#endif

/*
---------------------------------Normal distribution function---------------------------------
Approximates the amount the surface's microfacets are aligned to the halfway vector, 
influenced by the roughness of the surface; 
this is the primary function approximating the microfacets.
*/

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

/*
---------------------------------Geometry function--------------------------------- 
Describes the self-shadowing property of the microfacets. 
When a surface is relatively rough, the surface's microfacets 
can overshadow other microfacets reducing the light the surface reflects.
*/

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

/*
---------------------------------Fresnel equation--------------------------------- 
The Fresnel equation describes the ratio of surface reflection at different surface angles.
*/

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    //return F0 + (1.0 - F0) * pow(2, -5.55473 * cosTheta - 6.98316 * cosTheta);
    return (F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f));
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    float3 r = 1.0f - roughness;
    //return F0 + (max(r, F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
    return F0 + (max(r, F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}


/*
---------------------------------Light Calculations---------------------------------
*/

//Calculates the radiance of a Pointlight
float3 DoPointlight(Light L, PixelIn input, float3 normal)
{
    float3 VL = L.position.xyz - input.worldPos.xyz;
    float distance = length(VL);
    VL = normalize(VL);
    
    float3 N = normalize(normal);
    
    float3 diff = L.color.xyz;
    
    float diffuseFactor = max(dot(VL, N), 0.0f);
    diff *= diffuseFactor;
    float3 radiance = 0;
    if (diffuseFactor <= 0.)
    {
        radiance = diff; //multiply here with shadowCoeff if shadows
    }
    else
    {
        float D = max(distance - L.range, 0.f);
        float denom = D / L.range + 0.75f;
        float attenuation = 1.f / (denom * denom);
        float cutoff = 0.01f;
        
        attenuation = (attenuation - cutoff) / (1 - cutoff);
        attenuation = max(attenuation, 0.f);
        
        diff *= attenuation;
        
        radiance = diff; //multiply here with shadowCoeff if shadows
    }
    
    radiance *= L.intensity;
    return radiance;
}

//Calculates the radiance of a Direction Light
float3 DoDirectionlight(Light L, float3 normal)
{
    float3 N = normalize(normal);
    float3 VL = -normalize(L.direction.xyz);
    
    float3 diff = L.color.xyz;
    
    float diffuseFactor = max(dot(VL, N), 0.0f);
    
    diff *= diffuseFactor;
    
    float3 radiance = diff; //multiply here with shadowCoeff if shadows
    return radiance * L.intensity;
}

//Calculates the outgoing radiance level of each light
void CalcRadiance(PixelIn input, float3 V, float3 N, float roughness, float metallic, float3 albedo, float3 lightPos, float3 radiance, float3 F0, out float3 rad)
{   
    //Calculate Light Radiance
    float3 lightDir = normalize(lightPos - input.worldPos.xyz);
    float3 H = normalize(V + lightDir);
    float distance = length(lightPos - input.worldPos.xyz);
    
    //Cook-Torrance BRDF
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, lightDir, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);
    
    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= (1.0f - metallic);
    
    float3 nom = D * G * F;
    float denom = 4 * max(dot(N, V), 0.0f) * max(dot(N, lightDir), 0.0f) + 0.001f;
    float3 specular = nom / denom;
    
    float NdotL = max(dot(N, lightDir), 0.0f);
    rad = (((kD * albedo / PI) + specular) * radiance * NdotL);
}

float3 ambientIBL(float3 albedo, float3 N, float3 V, float3 F0, float metallic, float roughness, float ao)
{
    //Reflection Vector
    float3 R = reflect(-V, N);
    
    float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);
    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= (1.0f - metallic);
    
    float3 irradiance = t_irradiance.Sample(s_linear, N).rgb * c_tint;
    float3 diffuse = albedo * irradiance;
    
    const float MAX_REF_LOD = 4.0f;
    float3 prefilteredColor = t_radiance.SampleLevel(s_linear, R, roughness * MAX_REF_LOD).rgb * c_tint;
    float2 brdf = t_BRDFLUT.Sample(s_linear, float2(max(dot(N, V), 0.0f), roughness)).rg;
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    return (kD * diffuse + specular) * ao;
}