#include "common.hlsli"

//64 threads per group - best performance on all platforms
[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float4 finalColor = 0.f;
    
    //Box Blur
    if (c_blurType == 1)
    {
        int upper = ((c_blurRadius - 1) / 2);
        int lower = -upper;
        for (int x = lower; x <= upper; x++)
            for (int y = lower; y <= upper; y++)
                finalColor += t_bufferRead[DTid.xy + float2(x, y)];
    
        finalColor /= (c_blurRadius * c_blurRadius);
    }
    
    //Guassian Blur
    else if (c_blurType == 2)
    {
        float2 offset = 0.f;
        float weight = 0.f;
        
        if (c_useVertical)
            offset = float2(1.f, 0.f);
        else
            offset = float2(0.f, 1.f);
    
        //Go through the full array
        for (uint pos = 0; pos <= c_blurRadius; pos++)
        {
        //Unpackage the correct way from float4
            weight = (c_weights[pos / 4])[pos % 4];
            finalColor += t_bufferRead[DTid.xy + (offset * pos)] * weight;
        }
    
        //Go back through the array and ignore the main weight
        for (uint neg = c_blurRadius; neg > 0; neg--)
        {
            weight = (c_weights[neg / 4])[neg % 4];
            finalColor += t_bufferRead[DTid.xy + (offset * pos)] * weight;
        }

    }
    
    t_bufferOut[DTid.xy] = finalColor;
}