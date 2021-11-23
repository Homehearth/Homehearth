#ifndef _COMMON_HLSLI_
	#error You may not include this header directly.
#endif

float4 ClipToView(const float4 clip)
{
    // View space position.
    float4 view = mul(c_inverseProjection, clip);

    // Perspective projection.
    view = view / view.w;

    return view;
}

float4 ScreenToView(float4 screen)
{
    // Convert to normalized texture coordinates.
    const float2 texCoord = screen.xy / c_screenDimensions;

    // Convert to clip space.
	const float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);

    return ClipToView(clip);
}

Plane ComputePlane(float3 p0, float3 p1, float3 p2)
{
    Plane plane;

    // Compute a plane from 3 non-collinear points that form a triangle.
	// Assuming Counter-Clockwise Winding Order.
    const float3 v0 = p1 - p0; // C-A.
    const float3 v2 = p2 - p0; // C-A.

    plane.normal = normalize(cross(v0, v2));

    // Compute the distance to the origin using p0.
    plane.distanceToOrigin = dot(plane.normal, p0);

    return plane;
}

bool SphereInsidePlane(Sphere sphere, Plane plane)
{
    return dot(plane.normal, sphere.center) - plane.distanceToOrigin < -sphere.radius;
}

bool SphereInsideFrustum(Sphere sphere, Frustum frustum, float zNear, float zFar)
{
    bool result = true;

    // First check depth
    // Note: Here, the view vector points in the -Z axis so the 
    // far depth value will be approaching -infinity.
    if (sphere.center.z - sphere.radius > zNear || sphere.center.z + sphere.radius < zFar)
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