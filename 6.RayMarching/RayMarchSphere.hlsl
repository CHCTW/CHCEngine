#define HLSL
#include "RayMarch.hlsli"

#define CLOSE_EPSLION 0.001

RWTexture2D<float4> frame_buffer : register(u0);

[numthreads(16, 16, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    float3 center = float3(0, 0, -20);
    float radius = 6.0f;
    uint2 dim;
    frame_buffer.GetDimensions(dim.x, dim.y);
    if (DTid.x < dim.x && DTid.y < dim.y)
    {
    
        float3 dir = rayDirection(DTid.xy, dim);
        frame_buffer[DTid.xy] = float4(0.0, 0.0, 0.0, 1.0);
        float3 pos = float3(0, 0, 0);
        uint step = 0;
        float dis = 0;
        for (; step < MAXIMUM_TRACE_STEP; ++step)
        {
            if (abs(pos.x) > dim.x || abs(pos.y) > dim.y || abs(pos.z) > 200)
                break;
            dis = signedDistanceSphereFunction(pos, center, radius);
            if (dis < CLOSE_EPSLION)
                break;
            pos += dis * dir;
        }
        if (dis < CLOSE_EPSLION)
        {
        
            frame_buffer[DTid.xy] = float4(0.3, 0.3, 0.3, 1.0);
            //float value = float(step) / MAXIMUM_TRACE_STEP;
            float3 normal = estimateNormal(pos, center, radius);
            frame_buffer[DTid.xy] = float4(normal, 0.0);
        }
        else
        {
          
            
            float value = pow(float(step) / MAXIMUM_TRACE_STEP, 2) * 10.0;
            frame_buffer[DTid.xy] = float4(value, value, value, value);
            
            //frame_buffer[DTid.xy] = float4(dir, 0.0);
        }


    }
}
