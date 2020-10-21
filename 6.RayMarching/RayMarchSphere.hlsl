#define HLSL
#include "RayMarch.hlsli"

#define CLOSE_EPSLION 0.0001

RWTexture2D<float4> frame_buffer : register(u0);

[numthreads(16, 16, 1)]



void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    float3 center = float3(0.0, -2.0, -0.5);
    float radius = 0.4f;
    uint2 dim;
    frame_buffer.GetDimensions(dim.x, dim.y);
    if (DTid.x < dim.x && DTid.y < dim.y)
    {
    
        float3 dir = rayDirection(DTid.xy, dim);
        frame_buffer[DTid.xy] = float4(0.0, 0.0, 0.0, 1.0);
        float3 pos = float3(0, 0, 0);
        uint step = 0;
        float dis = 0;
        if (dir.x > 0)
            center.x = 0.5;
        else
            center.x = -0.5;
        for (; step < MAXIMUM_TRACE_STEP; ++step)
        {
            if (abs(pos.z) > 2000)
                break;
            float3 fold = pos;
            fold.xz = fmod(fold.xz, float2(1.0, 1.0));
            dis = signedDistanceSphereFunction(fold, center, radius);
            if (dis < CLOSE_EPSLION)
                break;
            pos += dis * dir;
        }
        if (dis < CLOSE_EPSLION)
        {
        
            frame_buffer[DTid.xy] = float4(0.3, 0.3, 0.3, 1.0);
            //float value = float(step) / MAXIMUM_TRACE_STEP;
            float3 fold = pos;
            fold.xz = fmod(fold.xz, float2(1.0, 1.0));

            
            float3 normal = estimateNormalsignedDistanceSphere(fold, center, radius);
            frame_buffer[DTid.xy] = float4(normal, 0.0);
        }
        else
        {
          
            
            float value = pow(float(step) / MAXIMUM_TRACE_STEP, 2) * 30.0;
            frame_buffer[DTid.xy] = float4(value, value, value, value);
            
            //frame_buffer[DTid.xy] = float4(dir, 0.0);
        }


    }
}
