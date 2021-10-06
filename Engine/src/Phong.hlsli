/*
 * This file contains helper functions used for the Phong light calculations. 
 */

struct Light
{
    float4 position;       
    float4 direction;      
    float4 color;          
    float specularPower;   
    float shininess;       
    float intensity;       
    float range;           
    uint enabled;          
    uint type;             
};


float ComputeAttenuation(float dist, float maxRange)
{
	// Smoothstep() returns 0 when the distance to the light is less than 3/4.
    return 1.0f - smoothstep(maxRange * 0.75f, maxRange, dist);
}

float4 ComputeDiffuse(float4 fragmentColor, float3 toLight, float3 N)
{
    const float diffuseFactor = saturate(dot(N, toLight));
    return fragmentColor * diffuseFactor;
}

float4 ComputeSpecular(float4 lightColor, float shininess, float specularPower, float3 toEye, float3 toLight, float3 N)
{
    const float3 reflection = reflect(-toLight, N);
    float specularFactor = saturate(dot(toEye, reflection));
    if (shininess > 1.0f)
    {
        specularFactor = pow(specularFactor, shininess);
    }
	
    return lightColor * specularFactor * specularPower;
}


void PointLightPhong()
{
	// todo:
}

void SpotLightPhong()
{
	// todo:
}

void DirectionalLightPhong()
{
	// todo:
}