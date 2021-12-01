#include "Common.hlsli"

[numthreads(32, 32, 1)]
void main(uint3 id : SV_DispatchThreadID)
{

    //Color compare
    float4 upperClamp = float4(0.65, 0.65, 0.65, 1);
    float4 lowerClamp = float4(0.7, 0.7, 0.7, 1);
    
    //Dimention
    float2 dimensions = { 0, 0 };
    t_waterTexture.GetDimensions(dimensions.x, dimensions.y);
   
    // Get the current pixel's normalized pos
    uint2 uv = id.xy;
    uint2 uv2 = id.xy;
    uint2 uv3 = id.xy;
  
    uint frequency = 25;
    uint amplitude = 5;
    
    //Wave
    uv.y += sin(uv.x * frequency) * amplitude * sin(c_counter + 10);
    uv.x += cos(uv.y * frequency) * amplitude * cos(c_counter + 10);
    
    uv3.y += cos(uv3.x * frequency) * amplitude * sin(c_counter + 10);
    uv3.x += sin(uv3.y * frequency) * amplitude * cos(c_counter + 10);

    //Move
    uv.y += uv.y + (c_counter * 50);
    
    uv2.y += uv2.y + (c_counter * 50);
    uv2.x += uv2.x + (c_counter * 20);
    
    uv3.y += uv3.y + (c_counter * 60);
    
    //Make sure it tiles. 
    uv.x = uv.x % dimensions.x;
    uv.y = uv.y % dimensions.y;
    
    uv2.x = uv2.x % dimensions.x;
    uv2.y = uv2.y % dimensions.y;
    
    uv3.x = uv3.x % dimensions.x;
    uv3.y = uv3.y % dimensions.y;
    
    //Get the pixel at said destination.
    float4 color  = t_waterTexture[uv.xy];
    float4 colorN = t_waterTextureN[uv2.xy];
    float4 colorB = t_waterBlend[uv3.xy];
    
    u_waterFloorTexture[id.xy] = colorB;
    
    ////Puts the color on the current pixel. 
    
    float4 tempColor = t_waterTexture[uv.xy];
    
    if (u_waterFloorTexture[id.xy].r >= upperClamp.r || u_waterFloorTexture[id.xy].g >= upperClamp.g || u_waterFloorTexture[id.xy].b >= upperClamp.b)
    {
        color.r = color.r + ( u_waterFloorTexture[id.xy].r * 0.7 );
        color.g = color.g + ( u_waterFloorTexture[id.xy].g * 0.7 );
        color.b = color.b + ( u_waterFloorTexture[id.xy].b * 0.7 );
        
        if (u_waterFloorTexture[id.xy].r <= lowerClamp.r || u_waterFloorTexture[id.xy].g <= lowerClamp.g || u_waterFloorTexture[id.xy].b <= lowerClamp.b)
        {
            color.r = tempColor.r + (u_waterFloorTexture[id.xy].r * 0.3);
            color.g = tempColor.g + (u_waterFloorTexture[id.xy].g * 0.3);
            color.b = tempColor.b + (u_waterFloorTexture[id.xy].b * 0.3);
        }
    }
    
    u_waterTexture[id.xy] = color;
    u_waterTextureN[id.xy] = colorN;
}
