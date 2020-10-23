#define HLSL
#include "RayMarch.hlsli"

#define CLOSE_EPSLION 0.01

RWTexture2D<float4> frame_buffer : register(u0);
cbuffer Time : register(b0)
{
    float time;
};

[numthreads(16, 16, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2 dim;
    frame_buffer.GetDimensions(dim.x, dim.y);
    float2 center = float2(-0.5, 0.1);
    float scale = 5;
    float looptime = 30;
    if (DTid.x < dim.x && DTid.y < dim.y)
    {
        float2 uv = DTid.xy;
        uv /= dim.xy;
        uv -= float2(0.5, 0.5);
        // base from -2~2
        uv *= float2(4.0, 4.0);
        uv /= float2(scale, scale);
        uv += center;
        float2 z0 = uv;
        float2 c1 = float2(-0.4, 0.6);
        float2 c2 = float2(-0.70176, 0.3842);
        float l = smoothstep(0, 1.0, (abs(time % (looptime * 2) - looptime)) / looptime);
        float2 c = lerp(c1, c2, l);
        frame_buffer[DTid.xy] = float4(0.0, 0.0, 0.0, 0.0);
        for (int i = 0; i < 500; ++i)
        {
            z0 = float2(z0.x * z0.x - z0.y * z0.y, 2.0 * z0.x * z0.y);
            z0 += c;
            if (dot(z0, z0) >= 4.0)
            {
                float f = i;
                f /= 250;
                f += 0.0333;
                f = frac(f);
                float3 color = HUEtoRGB(f) * f;
                frame_buffer[DTid.xy] = float4(color, time);
                return;
            }
        }
        frame_buffer[DTid.xy] = float4(0.2, 0.3, 0.6, time);
        


    }
}