Texture2D<float> SrcTexture : register(t0, space1);
RWTexture2D<float> DstTexture : register(u0, space1);

[numthreads(8,8,1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint width, height;
    DstTexture.GetDimensions(width, height);
    if (id.x >= width || id.y >= height) return;

    float depth = SrcTexture.Load(int3(id.xy, 0)).x;
    DstTexture[id.xy] = depth;
}
