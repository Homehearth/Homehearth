
/*
---------------------------------Shader Struct & Const Variables---------------------------------
*/

struct Light
{
    float4 position;    //Only in use on Point Lights
    float4 direction;   //Only in use on Directional Lights
    float4 color;       //Color and Intensity of the Lamp
    float  range;       //Only in use on Point Lights
    int    type;        // 0 = Directional, 1 = Point
    uint   enabled;     // 0 = Off, 1 = On
    float  padding;
};

struct PixelIn
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BINORMAL;
    float4 worldPos : WORLDPOSITION;
};

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

float3 FresnelSchlickRoughness(float cosTheta, float F0, float roughness)
{
    return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) 
    * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
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