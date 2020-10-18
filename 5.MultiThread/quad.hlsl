#include "QuadHLSLCompat.h"

StructuredBuffer<Position> positions : register(t0);
StructuredBuffer<CircleColor> colors : register(t1);
cbuffer IndexConstant : register(b0)
{
    uint index;
};
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : PS_UV;
    float3 pcolor : COLOR;
};
PSInput VSMain(float2 position : POSITION, float2 uv : UV)
{
    PSInput res;
    res.position = float4(position, 0.0, 1.0);
    res.position.x /= 50.0f;
    res.position.y /= 50.0f;
    res.position.x += positions[index].x_;
    res.position.y += positions[index].y_;
    res.position.z = positions[index].z_;
    res.pcolor = float3(colors[index].r_, colors[index].g_, colors[index].b_);
    res.uv = uv;
    return res;
}
float4 PSMain(PSInput input) : SV_TARGET
{
    float l = length(input.uv - float2(0.5, 0.5));
    l = step(l, 0.5f);
    if (l == 0.0)
        discard;
    return float4(input.pcolor, 1.0);
}