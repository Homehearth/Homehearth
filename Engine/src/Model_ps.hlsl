
//Texture2D diffuseTexture : register(t0);
SamplerState samp : register(s0);

struct PixelIn
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BINORMAL;
    float4 worldPos : WORLDPOSITION;
};


const float PI = 3.14159265359;

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
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float4 main(PixelIn input) : SV_TARGET
{
    float3 camPos = float3(0.0, 0.0, 3.0); //TODO: Get actual camera pos
    float3 albedo = float3(0.5, 0.0, 0.0); //TODO: Get albedo from texture
    float metallic = 0.1; //TODO: Get metallic from texture
	float roughness = 0.5; //TODO: Get roughness from texture
	float ao = 1.0; //TODO: Get ao from texture
    float3 lightPos = float3(0.0, 0.0, 0.0); //TODO: Light-struct to GPU
    float3 lightCol = float3(300.0, 300.0, 300.0); //TODO: Light-struct to GPU

    float3 N = normalize(input.normal);
    float3 V = normalize(camPos - input.worldPos.xyz);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
	float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    //Reflectance Equation
    float3 Lo = float3(0.0, 0.0, 0.0);
	
       //Calculate per-light radiance
	float3 L = normalize(lightPos - input.worldPos.xyz);
	float3 H = normalize(V + L);
	float distance = length(lightPos - input.worldPos.xyz);
	float attenuation = 1.0 / (distance * distance);
	float3 radiance = lightCol * attenuation;    
    
       //Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	float3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
    
	float3 numerator = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
	float3 specular = numerator / denominator;
    
       //kS is equal to Fresnel
	float3 kS = F;
       //For energy conservation, the diffuse and specular light can't
       //be above 1.0 (unless the surface emits light); to preserve this
       //relationship the diffuse component (kD) should equal 1.0 - kS.
	float3 kD = float3(1.0, 1.0, 1.0) - kS;
       //Multiply kD by the inverse metalness such that only non-metals
       //have diffuse lighting, or a linear blend if partly metal (pure metals
       //have no diffuse light).
	kD *= 1.0 - metallic;
    
       //Scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);
    
       //Add to outgoing radiance Lo
	Lo += (kD * albedo / PI + specular) * radiance * NdotL; //Note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	
    //Ambient lighting
    float3 ambient = float3(1.0, 1.0, 1.0) * albedo * ao;
    
    float3 color = ambient + Lo;
    
    //HDR tonemapping
	//color = color / (color + float3(1.0, 1.0, 1.0));
    //Gamma correct
	//color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
	float4 ReturnColor = float4(color, 1.0);
	return ReturnColor;
}