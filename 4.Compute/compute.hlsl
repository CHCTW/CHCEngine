#define THREADSIZE 8
RWTexture2D<float4> texts : register(u0);
/*cbuffer SceneConstantBuffer : register(b0)
{
    uint frame;
}*/

[numthreads(THREADSIZE, THREADSIZE, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2 dim;
    texts.GetDimensions(dim.x,dim.y);
    if (DTid.x<dim.x&&DTid.y<dim.y)
    {
        float2 uv = DTid.xy;
        uv /= (dim.xy - uint2(1, 1));
        texts[DTid.xy] = float4(uv.xy, 0.0, 1.0);
    }

}