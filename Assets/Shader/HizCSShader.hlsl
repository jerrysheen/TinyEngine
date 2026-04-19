RWTexture2D<float> gInputTexture  : register(u0, space1);
RWTexture2D<float> gOutputTexture : register(u1, space1);

[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint inputWidth, inputHeight;
    gInputTexture.GetDimensions(inputWidth, inputHeight);

    uint outputWidth, outputHeight;
    gOutputTexture.GetDimensions(outputWidth, outputHeight);

    if (DTid.x >= outputWidth || DTid.y >= outputHeight)
    {
        return;
    }

    uint2 coord = DTid.xy * 2;

    uint2 maxCoord = uint2(inputWidth - 1, inputHeight - 1);
    uint2 coord00 = min(coord, maxCoord);
    uint2 coord10 = min(coord + uint2(1, 0), maxCoord);
    uint2 coord01 = min(coord + uint2(0, 1), maxCoord);
    uint2 coord11 = min(coord + uint2(1, 1), maxCoord);

    float d00 = gInputTexture[coord00];
    float d10 = gInputTexture[coord10];
    float d01 = gInputTexture[coord01];
    float d11 = gInputTexture[coord11];

    // 如果 Reverse-Z 是 near=1 far=0，则 HiZ 保守值通常取 max。
    // 如果你当前 depth 约定真的是 0 最近 1 最远，才取 min。
    float hizDepth = min(min(d00, d10), min(d01, d11));

    gOutputTexture[DTid.xy] = hizDepth;
}
