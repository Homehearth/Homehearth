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
