// SimpleTestShader.hlsl - 简化版测试着色器

#include "include/Core.hlsl"




// 纹理资源
Texture2D g_Textures[1024] : register(t0, space0);


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
    nointerpolation uint dbgVID : TEXCOORD7;
    nointerpolation uint dbgIID : TEXCOORD8;
};

// 顶点着色器
VertexOutput VSMain(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VertexOutput output;


    // // 变换到世界空间
    uint index = g_VisibleInstanceIndices[instanceID + g_InstanceBaseOffset];
    PerObjectData data = g_InputPerObjectDatas[index];

    Vertex input = MeshBuffer[vertexID + data.baseVertexLocation];

    float4 worldPos = mul(float4(input.Position, 1.0f), data.objectToWorld);
    output.WorldPos = worldPos.xyz;
    
    // 变换到投影空间
    float4 viewPos = mul(worldPos, ViewMatrix);
    //output.Position = mul(worldPos, VPMatrix);
    output.Position = mul(viewPos, ProjectionMatrix);
    
    // 变换法向量
    output.Normal = normalize(mul(input.Normal, (float3x3)data.objectToWorld));
    output.Tangent = float4(normalize(mul(input.Tangent.xyz, (float3x3)data.objectToWorld)), input.Tangent.w);
    
    // 传递纹理坐标和颜色
    //output.TexCoord = input.TexCoord * TilingFactor;
    output.TexCoord = input.TexCoord;
    output.index = index;
    output.dbgVID = vertexID;
    output.dbgIID = instanceID;
    return output;
}

// 像素着色器
float4 PSMain(VertexOutput input) : SV_Target
{

    // 变换到世界空间
    PerObjectData data = g_InputPerObjectDatas[input.index];
    PerMaterialData matData = LoadPerMaterialData(data.matIndex);

    float2 uv = input.TexCoord;
    float4 diffuseSample = g_Textures[matData.DiffuseTextureIndex].Sample(LinearSampler, uv);
    float3 albedo = diffuseSample.xyz * matData.DiffuseColor.xyz;
    float3 emissive = g_Textures[matData.EmissiveTextureID].Sample(LinearSampler, uv).xyz;

    float4 specGlossSample = g_Textures[matData.MetallicTextureID].Sample(LinearSampler, uv);
    float3 specularColor = specGlossSample.rgb * matData.SpecularColor.xyz;
    float glossiness = saturate(specGlossSample.a * (1.0f - matData.Roughness));
    float roughness = saturate(1.0f - glossiness);

    float3 normalTS = g_Textures[matData.NormalTextureIndex].Sample(LinearSampler, uv).xyz * 2.0f - 1.0f;
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
    float3 direct = (diffuse + specular) * 1.5;
    //float3 ambient = albedo * AmbientColor * AmbientStrength;
    float3 ambient = albedo * 1 * 0.1;

    //float3 color = direct + ambient + emissive;
    float3 color = direct + ambient;
    return float4(color, 1.0f);
}