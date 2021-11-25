#include "Common.hlsli"

RWTexture2D<float4> Result;

[numthreads(32, 32, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    float mySin;
    
    //Green
    float4 newVector2 = float4(0, 0, 0, 1);
    
    //Displacement
    uint2 discplacement = { 10, 0 };

    //Dimention
    float2 dimentions = { 0, 0 };
    t_waterFloorTexture.GetDimensions(dimentions.x, dimentions.y);
   
    // Get the current pixel's normalized pos
    uint2 uv = id.xy;
  
    //Wave
    uv.y += cos(uv.x * 25 + (c_counter + 10)) * 10;

    
    //Make sure it tiles. 
    uv.x = uv.x % dimentions.x;
    uv.y = uv.y % dimentions.y;
    
    //Get the pixel at said destination.
    float4 color = t_waterFloorTexture[uv.xy];
    
    //Puts the color on the current pixel. 
    u_waterFloorTexture[id.xy] = color;
    
}
