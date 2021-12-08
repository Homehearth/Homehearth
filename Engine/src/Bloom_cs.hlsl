#include "common.hlsli"

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float4 pixel = t_bufferRead[DTid.xy];

    // Test bloom XD
    if (pixel.x > 0.8f & pixel.y > 0.8f & pixel.z > 0.8f)
    {
        t_bufferRead[DTid.xy] = float4(1.0f, 1.0f, 1.0f, 1.0f);
        float4 finalColor = 0.0f;
        
        int upper = 5;
        int lower = -upper;
        int count = 0;
        for (int x = lower; x <= upper; x++)
        {
            for (int y = lower; y <= upper; y++)
            {
                finalColor += t_bufferRead[DTid.xy + float2(x, y)];
                t_bufferRead[DTid.xy + float2(x, y)] = finalColor / count;
                count++;
            }
        }
    
        finalColor /= count;
        t_bufferRead[DTid.xy] = finalColor;
    }
}