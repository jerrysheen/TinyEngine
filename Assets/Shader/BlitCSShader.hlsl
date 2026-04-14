Texture2D<float> SrcTexture : register(t0, space1);
RWTexture2D<float> DstTexture : register(u0, space1);
SamplerState sampler_point_clamp : register(s0, space0);


[numthreads(8,8,1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint width, height;
    DstTexture.GetDimensions(width, height);
    if (id.x >= width || id.y >= height) return;
    float2 uv = float2(id.x + 0.5f, id.y + 0.5f) / float2(width, height);
    float depth = SrcTexture.SampleLevel(sampler_point_clamp, uv, 0).x;
    DstTexture[id.xy] = depth;
}