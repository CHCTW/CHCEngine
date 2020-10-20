#include "RayMarchHLSLCompt.h"
#define MAXIMUM_TRACE_STEP 250
#define NORMAL_EPSILION 0.0001f
float3 rayDirection(in float2 fragCoord, uint2 dim)
{
    float2 xy = float2(fragCoord.x, dim.y - fragCoord.y);
    
    xy = xy - float2(dim.x, dim.y) / 2.0;
    float z = float(dim.y) / tan(radians(90.0) / 2.0);
    return normalize(float3(xy, -z));
}
float signedDistanceSphereFunction(in float3 pos, in float3 center, float radius)
{
    float l = length(pos - center);
    return l - radius;
}

float3 estimateNormal(float3 p, in float3 center, float radius)
{
    return normalize(float3(
        signedDistanceSphereFunction(float3(p.x + NORMAL_EPSILION, p.y, p.z), center, radius) - signedDistanceSphereFunction(float3(p.x - NORMAL_EPSILION, p.y, p.z), center, radius),
        signedDistanceSphereFunction(float3(p.x, p.y + NORMAL_EPSILION, p.z), center, radius) - signedDistanceSphereFunction(float3(p.x, p.y - NORMAL_EPSILION, p.z), center, radius),
        signedDistanceSphereFunction(float3(p.x, p.y, p.z + NORMAL_EPSILION), center, radius) - signedDistanceSphereFunction(float3(p.x, p.y, p.z - NORMAL_EPSILION), center, radius)
    ));
}