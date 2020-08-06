#define THREADSIZE 8
RWTexture2D<float4> texts : register(u0);
struct Scene
{
    uint mouse_x;
    uint mouse_y;
    uint width;
    uint height;
};
cbuffer SceneConstantBuffer : register(b0)
{
    Scene scene;
}
float2 rand_2_10(in float2 uv)
{
    float noiseX = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    float noiseY = sqrt(1 - noiseX * noiseX);
    return float2(noiseX, noiseY);
}
[numthreads(THREADSIZE, THREADSIZE, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2 dim;
    texts.GetDimensions(dim.x,dim.y);
    if (DTid.x<dim.x&&DTid.y<dim.y)
    {
        
        float2 uv = float2(scene.mouse_x, scene.mouse_y);
        uv /= (uint2(scene.width, scene.height));
        uint dist = abs(DTid.y - uv.y * dim.y) * abs(DTid.y - uv.y * dim.y) + abs(DTid.x - uv.x * dim.x) * abs(DTid.x - uv.x * dim.x);
        float4 prev = texts[DTid.xy];
        float2 res = rand_2_10(uv * 5.0);
        if (dist<10)
            prev = float4(0.0,0.1, 0, 0);
        prev += float4(0.0, 0.001, 0.002, 0);
        prev = clamp(prev, float4(0.0, 0.0, 0, 0)
        ,float4(0.0, 0.5, 1.0, 0));

        texts[DTid.xy] = prev;
    }

}