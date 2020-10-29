#define HLSL
#include "RayMarch.hlsli"

#define CLOSE_EPSLION 0.001

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
    float3 center = float3(0.0, 0.0, 5.0);
    float scale = 5;
    float looptime = 2.0;
    if (DTid.x < dim.x && DTid.y < dim.y)
    {
        float2 uv = DTid.xy;
        uv /= dim.xy;
        uv -= float2(0.5, 0.5);
        float3 dir = rayDirection(DTid.xy, dim);
        dir.xy *= 0.75;
        dir = normalize(dir);
        float3 pos = float3(0, 0, 4.0);
        float4 c = float4(pos, 0.0);
        float start = 0.0;
       
        float4 c_array[5];
        c_array[0] = float4(-0.218, -0.113, -0.181, -0.496);
        c_array[1] = float4(-0.125, -0.256, 0.847, 0.0895);
        c_array[2] = float4(0.185, 0.478, 0.125, -0.392);
        c_array[3] = float4(-1, 0.2, 0, 0);
        
        float total_time = 5 * looptime;
        
        
        for (int i = 0; i < MAXIMUM_TRACE_STEP; ++i)
        {
            c_array[4] = float4(pos + start * dir, 0.0);
            float local_time = time % total_time;
            int segment = int(local_time) / int(looptime);
            float l = smoothstep(0.0, 1.0, fmod(local_time, looptime) / looptime);
            int next = (segment + 1) % 5;
            float4 c = lerp(c_array[segment], c_array[next], l);
            float h = singedDistanceFunction4DJulia(pos + start * dir, c);
            if (h < CLOSE_EPSLION)
            {
                pos = pos + start * dir;
                float f = i;
                f /= 500;
                //f += 0.3333;
                f = frac(f);
                float3 color = HUEtoRGB(f);
                float3 normal = estimateNormalsignedDistance4DJulia(pos, c);
                frame_buffer[DTid.xy] = float4(normal, time);
                float ao = AO4DJulia(pos, c, normal);
                
                
                float3 albedo = color;
                float metallic = 0.2f;
                float roughness = f;
            
                float3 F0 = float3(0.04, 0.04, 0.04);
                F0 = lerp(F0, albedo, float3(metallic, metallic, metallic));

                float3 N = normal;
                float3 V = normalize(-pos.xyz);
                float3 light_pos[2];
                float3 light_color[2];
                float itensity[2];
            
                light_pos[0] = float3(10.0, -50.0, 0.0);
                light_color[0] = float3(0.3, 0.3, 0.3);
                itensity[0] = 10.0;
            
                light_pos[1] = float3(30.0, 20.0, 10.0);
                light_color[1] = float3(1.0, 0.9, 0.8);
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
                
                
                
                frame_buffer[DTid.xy] = float4(pow(saturate(result + albedo * ao * 0.1), 1.0 / 2.2), time);
                return;
            }
            start += h;
        }
  
        frame_buffer[DTid.xy] = float4(0.0, 0.0, 0.0, time);
    }
}