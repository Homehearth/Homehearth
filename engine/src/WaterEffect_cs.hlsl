#include "Common.hlsli"

RWTexture2D<float4> Result;

[numthreads(32, 32, 1)]
void main(uint3 id : SV_DispatchThreadID)
{

    //Color compare
    float4 upperClamp = { 0.8, 0.8, 0.8, 1 };
    float4 lowerClamp = { 0.1, 0.1, 0.1, 1 };
    
    //Dimention
    float2 dimentions = { 0, 0 };
    t_waterTexture.GetDimensions(dimentions.x, dimentions.y);
   
    // Get the current pixel's normalized pos
    uint2 uv = id.xy;
    uint2 uv2 = id.xy;
  
    uint frequency = 25;
    uint amplitude = 5;
    
    //Wave
    uv.y += cos(uv.x * frequency + (c_counter + 10)) * amplitude;
    uv.x += cos(uv.y * frequency + (c_counter + 10)) * amplitude;

    //Move
    uv.y  += uv.y  + c_counter * 50;
    uv2.y += uv2.y + c_counter * 50;
    uv2.x += uv2.x + c_counter * 20;
    
    //Make sure it tiles. 
    uv.x  = uv.x % dimentions.x;
    uv.y  = uv.y % dimentions.y;
    
    uv2.x = uv2.x % dimentions.x;
    uv2.y = uv2.y % dimentions.y;
    
    //Get the pixel at said destination.
    float4 color  = t_waterTexture[uv.xy];
    float4 colorN = t_waterTextureN[uv2.xy];
    //float4 colorB = t_waterBlend[uv.xy];
    
    //u_waterBlend[id.xy] = colorB;
    
    ////Puts the color on the current pixel. 
    
    //if (u_waterBlend[id.xy].r >= upperClamp.r || u_waterBlend[id.xy].g >= upperClamp.g || u_waterBlend[id.xy].b >= upperClamp.b)
    //{
    //    color.r = 1;
    //    color.g = 1;
    //    color.b = 1;
    //}
    
    u_waterTexture[id.xy] = color;
    u_waterTextureN[id.xy] = colorN;
    
}
