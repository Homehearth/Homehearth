#include "common.hlsli"

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float4 finalColor = 0.f;
    int _KernelSize = 15;
    
    int upper = ((_KernelSize - 1) / 2);
    int lower = -upper;
    for (int x = lower; x <= upper; x++)
        for (int y = lower; y <= upper; y++)
            finalColor += t_bufferRead[DTid.xy + float2(x, y)];

    
    finalColor /= (_KernelSize * _KernelSize);
    
    t_bufferOut[DTid.xy] = finalColor;
}