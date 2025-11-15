// SimpleTestShader.hlsl - 简化版测试着色器

#include "include/Core.hlsl"


 // 材质相关的数据，先把Pass相关也放在这里
cbuffer PerMaterialData : register(b0, space2)
{
    float4 DiffuseColor;
    float4 SpecularColor;
    float Roughness;
    float Metallic;
    float2 TilingFactor;
}

// 每一个Drawcall都不同的数据
cbuffer PerDrawData : register(b0, space3)
{
    float3 CameraPosition;
    float4x4 ViewMatrix; 
    float4x4 ProjectionMatrix;
    float4x4 WorldMatrix;
    //float4x4 VPMatrix;
};

// 纹理资源
Texture2D DiffuseTexture : register(t0, space0);
Texture2D NormalTexture : register(t1, space0);
Texture2D SpecularTexture : register(t2, space0);
TextureCube EnvironmentMap : register(t3, space0);

// 采样器
SamplerState LinearSampler : register(s0, space0);
SamplerState PointSampler : register(s1, space0);
SamplerState AnisotropicSampler : register(s2,space0);
SamplerComparisonState ShadowSampler : register(s3, space0);

// 顶点着色器输入
struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

// 顶点着色器输出/像素着色器输入
struct VertexOutput
{
    float4 Position : SV_Position;
    float3 WorldPos : WORLD_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

// 顶点着色器
VertexOutput VSMain(VertexInput input)
{
    VertexOutput output;
    
    // 变换到世界空间
    float4 worldPos = mul(float4(input.Position, 1.0f), WorldMatrix);
    output.WorldPos = worldPos.xyz;
    
    // 变换到投影空间
    float4 viewPos = mul(worldPos, ViewMatrix);
    //output.Position = mul(worldPos, VPMatrix);
    output.Position = mul(viewPos, ProjectionMatrix);
    
    // 变换法向量
    output.Normal = normalize(mul(input.Normal, (float3x3)WorldMatrix));
    
    // 传递纹理坐标和颜色
    //output.TexCoord = input.TexCoord * TilingFactor;
    output.TexCoord = input.TexCoord;
    
    return output;
}

// 像素着色器
float4 PSMain(VertexOutput input) : SV_Target
{
    half4 diffuseColor = half4(0.0f, 0.0f,0.0f,1.0f);
    diffuseColor.xyz = DiffuseTexture.Sample(LinearSampler, input.TexCoord).xyz * AmbientColor;
    return diffuseColor;

    // // 采样纹理
    // float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    // float3 normalMap = NormalTexture.Sample(AnisotropicSampler, input.TexCoord).xyz;
    // float specularValue = SpecularTexture.Sample(PointSampler, input.TexCoord).r;
    
    // // 基础颜色
    // float3 albedo = diffuseColor.rgb * DiffuseColor.rgb;
    
    // // 简单光照计算
    // float3 normal = normalize(input.Normal);
    // float3 lightDir = normalize(-LightDirection);
    // float3 viewDir = normalize(CameraPosition - input.WorldPos);
    
    // // 漫反射
    // float NdotL = saturate(dot(normal, lightDir));
    // float3 diffuse = albedo * LightColor * LightIntensity * NdotL;
    
    // // 高光反射
    // float3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(saturate(dot(viewDir, reflectDir)), (1.0f - Roughness) * 128.0f);
    // float3 specular = SpecularColor.rgb * specularValue * spec * LightColor * LightIntensity;
    
    // // 环境光
    // float3 ambient = albedo * AmbientColor * AmbientStrength;
    
    // // 环境反射
    // float3 envReflect = reflect(-viewDir, normal);
    // float3 envColor = EnvironmentMap.Sample(LinearSampler, envReflect).rgb;
    // float3 fresnel = lerp(float3(0.04, 0.04, 0.04), albedo, Metallic);
    // float3 reflection = envColor * fresnel * (1.0f - Roughness);
    
    // // 最终颜色
    // float3 finalColor = ambient + diffuse + specular + reflection;
    
    // return float4(finalColor, diffuseColor.a);
}