Texture2D frame_buffer : register(t0);
SamplerState sample : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};
PSInput VSMain(uint id : SV_VertexID)
{
    PSInput result;
    result.uv = float2((id << 1) & 2, id & 2);
    result.position = float4(result.uv * float2(2, -2) + float2(-1, 1), 0, 1);


    return result;
}
float4 PSMain(PSInput input) : SV_TARGET
{

    return float4(frame_buffer.Sample(sample, input.uv));
}