#include "Common.hlsli"

[numthreads(32, 32, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    /*
         Water floor texture is the water blend texture. 
         It just uses the same UAV cuz of shader limitations. 
    */
    
    uint frequency = 25;
    uint amplitude = 5;
    
    //Color compare
    float4 upperClamp = float4(0.98, 0.98, 0.98, 1);
    float4 lowerClamp = float4(1, 1, 1, 1);
    
    //Dimention
    float2 dimensions = { 0, 0 };
    t_waterTexture.GetDimensions(dimensions.x, dimensions.y);
   
    // Get the current pixel's normalized pos
    uint2 uv  = id.xy;
    uint2 uv2 = id.xy;
    uint2 uv3 = id.xy;
    uint2 uv4 = id.xy;
    
    //Texture scaling
    uv.xy = uv.xy * 2;
    uv3.xy = uv3.xy * 3;
    uv4.xy = uv4.xy * 3;
    
    //Wave
    uv.y += sin(uv.x * frequency) * amplitude * sin(c_counter + 10);
    uv.x += cos(uv.y * frequency) * amplitude * cos(c_counter + 10);

    //Move
    uv.y += uv.y + (c_counter * 20);
    
    uv2.y += uv2.y + (c_counter * 20);
    uv2.x += uv2.y + (c_counter * 20);
    
    uv3.y += uv3.y + (c_counter * 60); //blend
    uv4.x += uv4.x + (c_counter * 60);
    
    //Make sure it tiles. 
    uv.x = uv.x % dimensions.x;
    uv.y = uv.y % dimensions.y;
    
    uv2.x = uv2.x % dimensions.x;
    uv2.y = uv2.y % dimensions.y;
    
    uv3.x = uv3.x % dimensions.x;
    uv3.y = uv3.y % dimensions.y;
    
    uv4.x = uv4.x % dimensions.x;
    uv4.y = uv4.y % dimensions.y;
    
    //Get the pixel at said destination.
    float4 color    = t_waterTexture[uv.xy];
    float4 colorN   = t_waterTextureN[uv2.xy];
    float4 colorB   = t_waterBlend[uv3.xy];
    float4 colorB2  = t_waterBlend[uv4.xy]; 
    
    //Save old color to not double light of edge if foam.
    float4 tempColor = t_waterTexture[uv.xy];
    
    //multiply the 2 blend textures
    colorB2 = colorB * colorB2 * 4;
    
    //Add foam on water
    if (colorB2.r >= upperClamp.r || colorB2.g >= upperClamp.g || colorB2.b >= upperClamp.b)
    {
        color.r = color.r + (colorB2.r * 0.7);
        color.g = color.g + (colorB2.g * 0.7);
        color.b = color.b + (colorB2.b * 0.7);
        
        //Add edge on foam. 
        if (colorB2.r <= lowerClamp.r || colorB2.g <= lowerClamp.g || colorB2.b <= lowerClamp.b)
        {
            color.r = tempColor.r + (colorB2.r * 0.3);
            color.g = tempColor.g + (colorB2.g * 0.3);
            color.b = tempColor.b + (colorB2.b * 0.3);
        }
    }
    
    u_waterFloorTexture[id.xy] = colorB2;
    u_waterTexture[id.xy] = color;
    u_waterTextureN[id.xy] = colorN;
}
