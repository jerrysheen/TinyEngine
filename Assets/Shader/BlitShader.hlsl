// BlitShader.hlsl
Texture2D SrcTexture : register(t0);
SamplerState LinearSampler : register(s0);

// 使用传统的顶点输入
struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;  
    float2 TexCoord : TEXCOORD0;
};

struct VertexOutput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

// 简化的顶点着色器 - 直接输出屏幕空间坐标
VertexOutput VSMain(VertexInput input)
{
    VertexOutput output;
    
    // input.Position已经是NDC坐标(-1到1)
    output.Position = float4(input.Position.xy, 0.0f, 1.0f);
    output.TexCoord = input.TexCoord;
    
    return output;
}

float4 PSMain(VertexOutput input) : SV_Target
{
    return SrcTexture.Sample(LinearSampler, input.TexCoord);
}