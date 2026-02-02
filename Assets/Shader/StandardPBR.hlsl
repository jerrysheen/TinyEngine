// SimpleTestShader.hlsl - 简化版测试着色器

#include "include/Core.hlsl"




// 纹理资源
//Texture2D g_Textures[1024] : register(t0, space0);
Texture2D DiffuseTexture : register(t0, space0);
Texture2D NormalTexture : register(t1, space0);
Texture2D MetallicTexture : register(t2, space0);
Texture2D EmissiveTexture : register(t3, space0);

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
    float4 Tangent : TANGENT;
};

// 顶点着色器输出/像素着色器输入
struct VertexOutput
{
    float4 Position : SV_Position;
    float3 WorldPos : WORLD_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Tangent : TANGENT;
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
    
    // 变换法向量/切线到世界空间
    output.Normal = normalize(mul(input.Normal, (float3x3)data.objectToWorld));
    output.Tangent = float4(normalize(mul(input.Tangent.xyz, (float3x3)data.objectToWorld)), input.Tangent.w);
    
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
    //float2 uv = input.TexCoord * matData.TilingFactor;
    float2 uv = input.TexCoord;
    float4 diffuseSample = DiffuseTexture.Sample(LinearSampler, uv);
    float3 albedo = diffuseSample.xyz * matData.DiffuseColor.xyz;
    float3 emissive = EmissiveTexture.Sample(LinearSampler, uv).xyz;

    float4 specGlossSample = MetallicTexture.Sample(LinearSampler, uv);
    float3 specularColor = specGlossSample.rgb * matData.SpecularColor.xyz;
    float glossiness = saturate(specGlossSample.a * (1.0f - matData.Roughness));
    float roughness = saturate(1.0f - glossiness);

    float3 normalTS = NormalTexture.Sample(LinearSampler, uv).xyz * 2.0f - 1.0f;
    normalTS = normalize(normalTS);

    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Tangent.xyz);
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T)) * input.Tangent.w;
    float3 normalWS = normalize(mul(normalTS, float3x3(T, B, N)));

    float3 L = normalize(-float3(-0.5,-0.5,1));
    float3 V = normalize(CameraPosition - input.WorldPos);
    float3 H = normalize(L + V);

    float NdotL = saturate(dot(normalWS, L));
    float NdotH = saturate(dot(normalWS, H));

    float3 F0 = specularColor;
    float specPower = lerp(32.0f, 8.0f, roughness);
    float3 specular = F0 * pow(NdotH, specPower);

    float3 diffuse = albedo * NdotL;
    //float3 direct = (diffuse + specular) * LightColor * LightIntensity;
    float3 direct = (diffuse + specular) * 1;
    //float3 ambient = albedo * AmbientColor * AmbientStrength;
    float3 ambient = albedo * 1 * 0.3;

    //float3 color = direct + ambient + emissive;
    float3 color = direct + ambient;
    return float4(color, 1.0f);

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
