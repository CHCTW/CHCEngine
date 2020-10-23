#include "RayMarchHLSLCompt.h"
#define MAXIMUM_TRACE_STEP 500
#define NORMAL_EPSILION 0.005f
#define AO_EPSILION 0.01f
#define AO_STEP 8
#define TETRAHEDRON_ITERATION 12
#define JULIA_ITERATION 12
#define PI 3.14159
float3 rayDirection(in float2 fragCoord, uint2 dim)
{
    float2 xy = float2(fragCoord.x, dim.y - fragCoord.y);
    
    xy = xy - float2(dim.x, dim.y) / 2.0;
    float z = float(dim.y) / tan(radians(90.0) / 2.0);
    if (z == 0.0)
    {
        z = 1.0;
    }
    return normalize(float3(xy, -z));
}
float signedDistanceSphereFunction(in float3 pos, in float3 center, float radius)
{
    float l = length(pos - center);
    return l - radius;
}

float3 estimateNormalsignedDistanceSphere(float3 p, in float3 center, float radius)
{
    return normalize(float3(
        signedDistanceSphereFunction(float3(p.x + NORMAL_EPSILION, p.y, p.z), center, radius) - signedDistanceSphereFunction(float3(p.x - NORMAL_EPSILION, p.y, p.z), center, radius),
        signedDistanceSphereFunction(float3(p.x, p.y + NORMAL_EPSILION, p.z), center, radius) - signedDistanceSphereFunction(float3(p.x, p.y - NORMAL_EPSILION, p.z), center, radius),
        signedDistanceSphereFunction(float3(p.x, p.y, p.z + NORMAL_EPSILION), center, radius) - signedDistanceSphereFunction(float3(p.x, p.y, p.z - NORMAL_EPSILION), center, radius)
    ));
}
// from shader toy : https://www.shadertoy.com/view/MdyBDc

float signedDistanceFunctionTetrahedron(float3 pos, float size, float scale, float3 center, float time)
{
    
    
    float3x3 rot = float3x3(
    	float3(cos(time * 0.5), 0.0, -sin(time * 0.5)),
        float3(0.0, 1.0, 0.0),
        float3(sin(time * 0.5), 0.0, cos(time * 0.5))
    );
    
    float3x3 rot2 = float3x3(
    	float3(cos(time * 0.5), -sin(time * 0.5), 0.0),
     float3(sin(time * 0.5), cos(time * 0.5), 0.0),
    float3(0.0, 0.0, 1.0)
    );
    float3 c1 = mul(rot,
    mul(rot2, float3(1, 1, 1) * size));
    float3 c2 = mul(rot, mul(rot2, float3(-1, -1, 1) * size));
    float3 c3 = mul(rot, mul(rot2, float3(1, -1, -1) * size));
    float3 c4 = mul(rot, mul(rot2, float3(-1, 1, -1) * size));
    pos = pos - center;
    int n = 0;
    float distance, d;
    float3 c;
    while (n < TETRAHEDRON_ITERATION)
    {
        c = c1;
        distance = length(pos - c1);
        d = length(pos - c2);
        if (d < distance)
        {
            distance = d;
            c = c2;
        }
        d = length(pos - c3);
        if (d < distance)
        {
            distance = d;
            c = c3;
        }
        d = length(pos - c4);
        if (d < distance)
        {
            distance = d;
            c = c4;
        }
        pos = scale * pos - c * (scale - 1.0);
        ++n;
    }
    return length(pos) * pow(scale, float(-n));

}
float3 estimateNormalsignedDistanceTetrahedron(float3 p, float size, float scale, float3 center, float time)
{
    return normalize(float3(
        signedDistanceFunctionTetrahedron(float3(p.x + NORMAL_EPSILION, p.y, p.z), size, scale, center, time) - signedDistanceFunctionTetrahedron(float3(p.x - NORMAL_EPSILION, p.y, p.z), size, scale, center, time),
        signedDistanceFunctionTetrahedron(float3(p.x, p.y + NORMAL_EPSILION, p.z), size, scale, center, time) - signedDistanceFunctionTetrahedron(float3(p.x, p.y - NORMAL_EPSILION, p.z), size, scale, center, time),
        signedDistanceFunctionTetrahedron(float3(p.x, p.y, p.z + NORMAL_EPSILION), size, scale, center, time) - signedDistanceFunctionTetrahedron(float3(p.x, p.y, p.z - NORMAL_EPSILION), size, scale, center, time)
    ));
}

float3 Fresnel(float3 F0, float HV)
{
    return F0 + (1.0 - F0) * pow(2, (-5.55473 * HV - 6.98316) * HV);
//	return F0 + (1.0 - F0)*pow(1 - HV, 5.0f);
}


float Distribution(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
float GeometrySchlickGGX(float NV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NV;
    float denom = NV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(float NV, float NL, float roughness)
{
    float ggx2 = GeometrySchlickGGX(NV, roughness);
    float ggx1 = GeometrySchlickGGX(NL, roughness);
    return ggx1 * ggx2;
}
float3 HUEtoRGB(in float H)
{
    float R = abs(H * 6 - 3) - 1;
    float G = 2 - abs(H * 6 - 2);
    float B = 2 - abs(H * 6 - 4);
    return saturate(float3(R, G, B));
}
float singedDistanceFunction4DJulia(float3 pos, float4 c)
{
    float4 z = float4(pos, 0.0);
    float mz2 = dot(z, z);
    float md2 = 1.0;
    float4 nz;
    for (int i = 0; i < JULIA_ITERATION; ++i)
    {
        md2 *= 4.0 * mz2;
        nz.x = z.x * z.x - dot(z.yzw, z.yzw);
        nz.yzw = 2.0 * z.x * z.yzw;
        z = nz + c;

        mz2 = dot(z, z);
        if (mz2 > 4.0)
        {
            break;
        }
    }
    return 0.25 * sqrt(mz2 / md2) * log(mz2);
}

float3 estimateNormalsignedDistance4DJulia(float3 p, float4 c)
{
    return normalize(float3(
        singedDistanceFunction4DJulia(float3(p.x + NORMAL_EPSILION, p.y, p.z), c) - singedDistanceFunction4DJulia(float3(p.x - NORMAL_EPSILION, p.y, p.z), c),
        singedDistanceFunction4DJulia(float3(p.x, p.y + NORMAL_EPSILION, p.z), c) - singedDistanceFunction4DJulia(float3(p.x, p.y - NORMAL_EPSILION, p.z), c),
        singedDistanceFunction4DJulia(float3(p.x, p.y, p.z + NORMAL_EPSILION), c) - singedDistanceFunction4DJulia(float3(p.x, p.y, p.z - NORMAL_EPSILION), c)
    ));
}
float AO4DJulia(float3 p, float4 c, float3 normal)
{
    float total = 0.0;
    for (int i = 0; i < AO_STEP; ++i)
    {
        total += ((i + 1) * AO_EPSILION - singedDistanceFunction4DJulia(p + normal * (i + 1) * AO_EPSILION, c)) / pow(2.0, i + 1);
    }
    return 1.0 - 10.0 * total;

}