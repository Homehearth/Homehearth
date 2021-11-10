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
    const float3 v0 = p1 - p0;
    const float3 v2 = p2 - p0;
    plane.normal = normalize(cross(v0, v2));
    plane.distanceToOrigin = dot(plane.distanceToOrigin, p0);

    return plane;
}