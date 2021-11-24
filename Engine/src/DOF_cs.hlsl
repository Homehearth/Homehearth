#include "common.hlsli"

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float width;
    float height;
    float4 finalColor = 0.f;
    t_inFocus.GetDimensions(width, height);
    float4 focusColor = t_inFocus[DTid.xy];
    float4 outOfFocusColor = t_outOfFocus[DTid.xy];
    
    if(c_dofType == 1)
    {
        float minDistance = 10.f;
        float maxDistance = 30.f;
    
        float4 position = ViewPosFromDepth(t_depth[DTid.xy].x, DTid.xy);
    
        width /= 2;
        height /= 2;
        height -= 50;
    
        float2 focus = float2(width, height);
        float4 focusPoint = ViewPosFromDepth(t_depth[focus].x, focus);
    
        float blur = smoothstep(minDistance, maxDistance, abs(position.z - focusPoint.z));
    
        finalColor = lerp(focusColor, outOfFocusColor, blur);
    }
    
    
    if(c_dofType == 2)
    {
        float inner = 0.1f;
        float outer = 1.2f;
        float strength = 0.8f;
        float curvature = 0.5f;
        
        float2 resolution = float2(width, height);
        float2 uv = DTid.xy / resolution;
    
    
        float2 curve = pow(abs(uv * 2 - 1), float2(1 / curvature, 1 / curvature));
        float edge = pow(length(curve), curvature);
        float vignette = 1 - strength * smoothstep(inner, outer, edge);
    
        finalColor = lerp(outOfFocusColor, focusColor, vignette);
    }  
    
    t_dofOut[DTid.xy] = finalColor;
}