//Texture2D diffuseTexture : register(t13);
//Texture2D dissplacementTexture : register(t14);
//SamplerState pointSampler : register(s1);
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
    
    UV displacement:
    https://gamedevelopment.tutsplus.com/tutorials/using-displacement-shaders-to-create-an-underwater-effect--cms-27191
*/

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    
    
    
}
