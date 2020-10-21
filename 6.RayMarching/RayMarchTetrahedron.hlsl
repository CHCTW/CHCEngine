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
    float3 center = float3(0.0, 0.0, -4.0);
    float scale = 2.0;
    float size = 1.0;
    if (DTid.x < dim.x && DTid.y < dim.y)
    {
        float3 dir = rayDirection(DTid.xy, dim);
        frame_buffer[DTid.xy] = float4(0.0, 0.0, 0.0, 1.0);
        float3 pos = float3(0, 0, 0);
        uint step = 0;
        float dis = 0;
        for (; step < MAXIMUM_TRACE_STEP; ++step)
        {
            if (abs(pos.z) > 2000)
                break;
            dis = signedDistanceFunctionTetrahedron(pos, size, scale, center, time);
            if (dis < CLOSE_EPSLION)
                break;
            pos += dis * dir;
        }
        if (dis < CLOSE_EPSLION)
        {
            frame_buffer[DTid.xy] = float4(0.3, 0.3, 0.3, 1.0);
            float3 normal = estimateNormalsignedDistanceTetrahedron(pos, size, scale, center, time);
            frame_buffer[DTid.xy] =
            float4(normal, 1.0);
            
            float3 albedo = float3(0.2, 0.3, 0.6);
            float metallic = 0.9f;

            float roughness = 0.3;
            
            float3 F0 = float3(0.04, 0.04, 0.04);
            F0 = lerp(F0, albedo, float3(metallic, metallic, metallic)); // use metalic value to get F

            float3 N = normal;
            float3 V = normalize(-pos.xyz);
            float3 light_pos[2];
            float3 light_color[2];
            float itensity[2];
            
            light_pos[0] = float3(80.0, 0.0, 80.0);
            light_color[0] = float3(0.3, 0.3, 0.7);
            itensity[0] = 20.0;
            
            light_pos[1] = float3(0.0, 0.0, -4.0);
            light_color[1] = float3(1.0, 0.3, 0.1);
            itensity[1] = 10.0;
            
            float3 result;
            
            for (int i = 0; i < 2; ++i)
            {
            
                float3 L = normalize(light_pos[i] - pos.xyz);
                float3 H = normalize(L + V);
                float LH = max(dot(L, H), 0.0f);
                float NL = max(dot(L, N), 0.0f);
                float NV = max(dot(N, V), 0.0f);
                float HV = max(dot(H, V), 0.0f);

                float3 F = Fresnel(F0, HV);
                float D = Distribution(N, H, roughness);
                float3 G = GeometrySmith(NV, NL, roughness);

                float3 spec = D * F * G / (4 * NV * NL + 0.001);

                float3 Kd = 1 - F; // diffuse color 
                Kd = Kd * (1.0 - metallic);
                float3 diff = Kd * albedo / PI;
                result += (diff + spec) * NL * light_color[i] * itensity[i];
            
            }
            frame_buffer[DTid.xy]
             = float4(albedo * 0.1, 0.0) +
            float4(result, time);
            //frame_buffer[DTid.xy] =float4(normal, time);
            

        }
        else
        {
            float value = pow(float(step) / MAXIMUM_TRACE_STEP, 2) * 60.0;
            frame_buffer[DTid.xy] = float4(value * 0.3, value, value, value);
        }
    }
}