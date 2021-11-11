#ifndef _COMMON_HLSLI_
	#error You may not include this header directly.
#endif

static const float PI = 3.14159265359;

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
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
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
    return F0 + (1.0 - F0) * pow(2, -5.55473 * cosTheta - 6.98316 * cosTheta);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


/*
---------------------------------Light Calculations---------------------------------
*/

//Calculates the radiance of a Pointlight
float3 DoPointlight(Light L, PixelIn input, float3 normal)
{
    float3 VL = normalize(L.position.xyz - input.worldPos.xyz);
    float distance = length(VL);
    
    float3 N = normalize(normal);
    
    float3 diff = float3(1.0f, 1.0f, 1.0f);
    
    float diffuseFactor = max(dot(VL, N), 0.0f);
    diff *= diffuseFactor;
    float3 radiance = 0;
    if(diffuseFactor <= 0.)
    {
        radiance = diff; //multiply here with shadowCoeff if shadows
    }
    else
    {
        float D = max(distance - L.range, 0.f);
        
        float denom = D / L.range + 0.75f;
        float attenuation = 1.f / (denom * denom);
        float cutoff = 0.1f;
        
        attenuation = (attenuation - cutoff) / (1 - cutoff);
        attenuation = max(attenuation, 0.f);
        
        diff *= attenuation;
        
        radiance = diff; //multiply here with shadowCoeff if shadows
    }   
    
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
    return radiance;
}

//Calculates the outgoing radiance level of each light
void CalcRadiance(PixelIn input, float3 V, float3 N, float roughness, float metallic, float3 albedo, float3 lightPos, float3 radiance, float3 F0, out float3 rad)
{
    static const float PI = 3.14159265359;
    
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
    kD *= (1.0 - metallic);
    
    float3 nom = D * G * F;
    float denom = 4 * max(dot(N, V), 0.0f) * max(dot(N, lightDir), 0.0f) + 0.001f;
    float3 specular = nom / denom;
    
    float NdotL = max(dot(N, lightDir), 0.0f);
    rad = (((kD * albedo / PI) + specular) * radiance * NdotL);
}

void SampleTextures(PixelIn input, inout float3 albedo, inout float3 N, inout float roughness, inout float metallic, inout float ao)
{
    //If albedo texture exists, sample from it
    if(c_hasAlbedo == 1)
    {
        float4 albedoSamp = t_albedo.Sample(s_linear, input.uv);
        //Alpha-test
        clip(albedoSamp.a < 0.5f ? -1 : 1);
        albedo = pow(max(albedoSamp.rgb, 0.0f), 2.2f); //Power the albedo by 2.2f to get it to linear space.
    }
    
    //If normal texture exists, sample from it
    if(c_hasNormal == 1)
    {
        float3 normalMap = t_normal.Sample(s_linear, input.uv).rgb;
        normalMap = normalMap * 2.0f - 1.0f;
        
        float3 tangent = normalize(input.tangent.xyz);
        float3 biTangent = normalize(input.biTangent);
        float3x3 TBN = float3x3(tangent, biTangent, input.normal);
        
        N = normalize(mul(normalMap, TBN));
    }
    
    //If metallic texture exists, sample from it
    if(c_hasMetalness == 1)
    {
        metallic = t_metalness.Sample(s_linear, input.uv).r;
    }
    
    //If roughness texture exists, sample from it
    if(c_hasRoughness == 1)
    {
        roughness = t_roughness.Sample(s_linear, input.uv).r;
    }
    
    //If ao texture exists, sample from it
    if(c_hasAoMap == 1)
    {
        ao = t_aomap.Sample(s_linear, input.uv).r;
    }
}

float4x4 inverse(float4x4 m)
{
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    float idet = 1.0f / det;

    float4x4 ret;

    ret[0][0] = t11 * idet;
    ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    ret[1][0] = t12 * idet;
    ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    ret[2][0] = t13 * idet;
    ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    ret[3][0] = t14 * idet;
    ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

    return ret;
}