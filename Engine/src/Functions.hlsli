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