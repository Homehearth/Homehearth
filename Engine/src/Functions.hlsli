#ifndef _COMMON_HLSLI_
	#error You may not include this header directly.
#endif

void SampleTextures(PixelIn input, inout float3 albedo, inout float3 N, inout float roughness, inout float metallic, inout float ao)
{
    //If albedo texture exists, sample from it
    if (c_hasAlbedo == 1)
    {
        float4 albedoSamp = t_albedo.Sample(s_linear, input.uv);
        //Alpha-test
        clip(albedoSamp.a < 0.5f ? -1 : 1);
        albedo = pow(max(albedoSamp.rgb, 0.0f), 2.2f); //Power the albedo by 2.2f to get it to linear space.
    }
    
    //If normal texture exists, sample from it
    if (c_hasNormal == 1)
    {
        float3 normalMap = t_normal.Sample(s_linear, input.uv).rgb;
        normalMap = normalMap * 2.0f - 1.0f;
        
        float3 tangent = normalize(input.tangent.xyz);
        float3 biTangent = normalize(input.biTangent);
        float3x3 TBN = float3x3(tangent, biTangent, input.normal);
        
        N = normalize(mul(normalMap, TBN));
    }
    
    //If metallic texture exists, sample from it
    if (c_hasMetalness == 1)
    {
        metallic = t_metalness.Sample(s_linear, input.uv).r;
    }
    
    //If roughness texture exists, sample from it
    if (c_hasRoughness == 1)
    {
        roughness = t_roughness.Sample(s_linear, input.uv).r;
    }
    
    //If ao texture exists, sample from it
    if (c_hasAoMap == 1)
    {
        ao = t_aomap.Sample(s_linear, input.uv).r;
    }
}


float4 ViewPosFromDepth(float depth, float2 texCoord)
{
    float z = depth * 2.0 - 1.0;
    
    float4 clipSpacePos = float4(texCoord * 2.0 - 1.0, z, 1.0);
    float4 viewSpacePosition = mul(c_inverseProjection, clipSpacePos);
    
    viewSpacePosition /= viewSpacePosition.w;
    
    return viewSpacePosition;
}

float4 WorldPosFromDepth(float depth, float2 texCoord)
{
    float z = depth * 2.0 - 1.0;
    
    float4 clipSpacePos = float4(texCoord * 2.0 - 1.0, z, 1.0);
    float4 viewSpacePosition = mul(c_inverseProjection, clipSpacePos);
    
    viewSpacePosition /= viewSpacePosition.w;
    
    float4 worldPos = mul(c_inverseView, viewSpacePosition);
    
    return worldPos;
}