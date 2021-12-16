#ifndef _COMMON_HLSLI_
	#error You may not include this header directly.
#endif

float4 ClipToView(float4 clip)
{
    // View space position.
    float4 view = mul(inverseProjection, clip);

    // Perspective projection.
    view = view / view.w;

    return view;
}

float4 ScreenToView(float4 screen)
{
    // Convert to normalized texture coordinates.
    const float2 texCoord = screen.xy / screenDimensions;

    // Convert to clip space.
    const float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);

    return ClipToView(clip);
}

Plane ComputePlane(float3 b, float3 c)
{
    Plane plane;
    // normalize(cross( b-a, c-a )), except we know "a" is the origin
	// also, typically there would be a fourth term of the plane equation, 
	// -(n dot a), except we know "a" is the origin
    plane.normal = normalize(cross(b, c));

    plane.distanceToOrigin = 0;

    return plane;
}

bool SphereInsidePlane(Sphere sphere, Plane plane)
{
    return dot(plane.normal, sphere.center) < -sphere.radius;
}

bool SphereInsideFrustum(Sphere sphere, Frustum frustum, float zNear, float zFar)
{
    bool result = true;

    // First check depth
    // Note: Switched zNear and zFar, because assuming left-handed coord.
    // far depth value will be approaching infinity.
    if (sphere.center.z - sphere.radius > zFar || sphere.center.z + sphere.radius < zNear) // OUTSIDE
    {
        result = false;
    }

    // Then check frustum planes
    for (int i = 0; i < 4 && result; i++)
    {
        if (SphereInsidePlane(sphere, frustum.planes[i]))
        {
            result = false;
        }
    }

    return result;
}

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


float4 ProjectionPosFromDepth(float depth, float2 texCoord)
{
    float z = depth * 2.0 - 1.0;
    float4 clipSpacePos = float4(texCoord * 2.0 - 1.0, z, 1.0);
    return clipSpacePos;
}


float4 ClipSpaceToParaboloidMapSpace(float4 pos, out float clipDepth)
{
    pos.xyz /= pos.w;
    float len = length(pos.xyz);
    pos /= len;
    
    clipDepth = pos.z;
    
    pos.z++;
    pos.xy /= pos.z;
    pos.z = (len - 0.1f) / (500.0f - 0.1f);
    if(pos.z > 0.0f)
    pos.w = 1.0f;
    return pos;
}


float SampleShadowMap(float2 texCoords, int shadowIndex, float currentDepth, int size, int kernalSize)
{
    float texelSize = 1.f / size;
    float shadowCoef = 0.0f;
	// blur
    int offset = (kernalSize / 2);
    
    for (int i = -offset; i <= offset; i++)
    {
        for (int j = -offset; j <= offset; j++)
        {
            float depth = t_shadowMaps.Sample(s_linear, float3(texCoords + float2(i, j) * texelSize, shadowIndex)).r;
            shadowCoef += depth < currentDepth ? 1.0f : 0.0f;
        }
    }

    shadowCoef /= kernalSize * kernalSize;
    return shadowCoef;
}

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 ACESFitted(float3 color)
{
    color = mul(ACESInputMat, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = mul(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

//void DilateBlur(RWTexture2D<unorm float4> t_bokehBufferRead, out RWTexture2D<unorm float4> t_bokehBufferOut, uint3 DTid)
//{
//    int size = 1;
//    float separation = 1.f;
//    float minThreshold = 0.1f;
//    float maxThreshold = 0.3f;
    
//    int width, height;
//    t_bokehBufferRead.GetDimensions(width, height);
//    float2 texSize = float2(width, height);
    
//    float2 texCoord = DTid.xy;
    
//    float4 color = t_bokehBufferRead[texCoord / texSize];
    
//    float mx = 0.f;
//    float4 cmx = color;
    
//    for (int i = -size; i <= size; i++)
//    {
//        for (int j = -size; j <= size; j++)
//        {
//            if (!(distance(float2(i, j), float2(0, 0)) <= size))
//            {
//                continue;
//            }
            
//            float4 c = t_bokehBufferRead[(texCoord.xy + (float2(i, j) * separation)) / texSize];
            
//            float mxt = dot(c.rgb, float3(0.3, 0.59, 0.11));
            
//            if (mxt > mx)
//            {
//                mx = mxt;
//                cmx = c;
//            }
//        }

//    }

    
//    t_bokehBufferOut[DTid.xy] = lerp(color, cmx, smoothstep(minThreshold, maxThreshold, mx));
//}