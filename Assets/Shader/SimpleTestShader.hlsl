// SimpleTestShader.hlsl - 简化版测试着色器

#include "include/Core.hlsl"




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
    nointerpolation int index : TEXCOORD1;
};

// 顶点着色器
VertexOutput VSMain(VertexInput input, uint instanceID : SV_InstanceID)
{
    VertexOutput output;
    // // 变换到世界空间
    uint index = g_VisibleInstanceIndices[instanceID + g_InstanceBaseOffset];
    PerObjectData data = g_InputPerObjectDatas[index];

    float4 worldPos = mul(float4(input.Position, 1.0f), data.objectToWorld);
    output.WorldPos = worldPos.xyz;
    
    // 变换到投影空间
    float4 viewPos = mul(worldPos, ViewMatrix);
    //output.Position = mul(worldPos, VPMatrix);
    output.Position = mul(viewPos, ProjectionMatrix);
    
    // 变换法向量
    output.Normal = normalize(mul(input.Normal, (float3x3)data.objectToWorld));
    
    // 传递纹理坐标和颜色
    //output.TexCoord = input.TexCoord * TilingFactor;
    output.TexCoord = input.TexCoord;
    output.index = index;
    return output;
}

// 像素着色器
float4 PSMain(VertexOutput input) : SV_Target
{

    // 变换到世界空间
    PerObjectData data = g_InputPerObjectDatas[input.index];
    PerMaterialData matData = LoadPerMaterialData(data.matIndex);
    half4 diffuseColor = matData.DiffuseColor;
    diffuseColor.xyz = DiffuseTexture.Sample(LinearSampler, input.TexCoord).xyz * diffuseColor.xyz ;
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