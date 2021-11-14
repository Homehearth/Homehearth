#include "common.hlsli"

VertexOutSB main(float3 skypos : SKYPOS)
{
    VertexOutSB output;
     
    //Coordinates to texture in 3d
    output.texCoord = skypos;
    
    float4x4 viewProj = mul(c_projection, c_view);
    
    //Sets the z = w. Makes z = 1 everytime z/w. Always as far back as it can be in viewfrustum.
    //w in skyPos is 0.0f as translation is not important - should be in the center of the camera
    output.posCS = mul(viewProj, float4(skypos, 0.0f)).xyww;
    
    return output;
}