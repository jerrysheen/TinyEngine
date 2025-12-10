// #define PER_FRAME_CB(name, slot)  cbuffer name : register(b##slot, space0)
// #define PER_MATERIAL_CB(name, slot)   cbuffer name : register(b##slot, space1)
// #define PER_DRAW_CB(name, slot) cbuffer name : register(b##slot, space2)

//cbuffer PerMaterialData : register(b0, space2)
//{
//    //float _FlipY;
//}


// BlitShader.hlsl
Texture2D SrcTexture : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

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
    float _FlipY = 1.0f;
    float2 uv = input.TexCoord;
    if(_FlipY > 0.1) uv.y = 1.0 - uv.y; 
    return SrcTexture.Sample(LinearSampler, uv);
}