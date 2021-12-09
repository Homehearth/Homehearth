#include "common.hlsli"

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float4 bloomMap = t_bufferRead[DTid.xy];
    float4 pixel = t_bufferOut[DTid.xy];
    pixel += bloomMap * 1.15f;
    //float4 output = (ACESFitted(pixel.xyz), 0.0f);
    t_bufferOut[DTid.xy] = pixel;

}