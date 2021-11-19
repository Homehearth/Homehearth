#include "common.hlsli"


[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float minDistance = 10.f;
    float maxDistance = 30.f;
    
    float4 focusColor = t_inFocus[DTid.xy];
    float4 outOfFocusColor = t_outOfFocus[DTid.xy];
    
    float4 position = ViewPosFromDepth(t_depth[DTid.xy].x, DTid.xy);
    
    float width;
    float height;
    uint mipLevels;
    t_depth.GetDimensions(0, width, height, mipLevels);
    width /= 2;
    height /= 2;
    
    float2 focus = float2(width, height);
    float4 focusPoint = ViewPosFromDepth(t_depth[focus].x, focus);
    
    float blur = smoothstep(minDistance, maxDistance, abs(position.z - focusPoint.z));
    
    
    float4 finalCol = lerp(focusColor, outOfFocusColor, blur);
    
    t_dofOut[DTid.xy] = finalCol;
}