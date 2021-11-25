
#include "Common.hlsli"

/*
    This shader will do the texture displacement. 
    It will use a ~Direction~ and a ~Distance~.
    If i use the UV:coordinates the size of the texture wont matter.. (Im not sure how to do that.)

    Okay scrap that, i will be using UV coordinates. Everytihng thats not about UV coordinates isn't relevant. 
    I will be using a sin curve instead. The following variables: ~Frequency~ and ~Amplitude~.

    InterlockedAdd function: https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/interlockedadd
    Gives two threads the access to write to the same memory by adding. 
       - This only works if the memory is read and writeable 
          (I assume they mean that it has to be a unordered access view kinda thing.)

    A method of pushing the pixels around: 
        - Instead of "moving" the pixel to its destination the pixel looks for
          the pixel that will replace it and copies it to the locations of itself to the new texture.
        - Every pixel (thread) keeps track of the pixel that might be copied to its location on the new texture,
          This is why Direction and Distance is needed.
        - A value between 0 and 1 can mean so and so many pixels: Distance / Value 0 - 1 (gray value) = new pixel placement. (I tihnk)
    
    https://gamedevelopment.tutsplus.com/tutorials/using-displacement-shaders-to-create-an-underwater-effect--cms-27191

How do do shit: 
    UV:s 
    - Structured buffer for reading. 
        - Sno vertexbuffern från någonstanns och skicka in den som en shader resours view och regestrera den som en structured buffer.

    Textures
    - Water normal map
    - Water map
    - Ground map
    - ground edge map.
        - Skicka in dessa som Unordered Acces Views.

    Delta time
    - cbuffer
        - Updateras med update subresource.
        - Skickas in som en constantbuffer.

    Mina andra variabler.
    - hårdkodas först. 
*/

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
