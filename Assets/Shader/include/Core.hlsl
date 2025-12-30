#ifndef CORE_HLSLI
#define CORE_HLSLI

cbuffer DrawIndices : register(b0, space0)
{
    uint g_InstanceBaseOffset;
}

cbuffer PerFrameData : register(b1, space0)
{
    float3 LightDirection;
    float LightIntensity;
    float3 LightColor;
    float padding;
    float3 AmbientColor;
    float AmbientStrength;
    float Time;
}

cbuffer PerPassData : register(b2, space0)
{
    float3 CameraPosition;
    float4x4 ViewMatrix; 
    float4x4 ProjectionMatrix;
}

// // 1. AABB 包围盒定义 (对应 C++ 结构体)
struct AABB
{
    float3 Min; // AABB 最小点
    float3 Max; // AABB 最大点
    float2 Padding;
};


struct PerObjectData
{
    float4x4 objectToWorld;
    uint matIndex;
    uint renderProxyStartIndex;
    uint renderProxyCount;
    uint padding; 
};


struct PerMaterialData
{
    float4 DiffuseColor;
    float4 SpecularColor;
    float Roughness;
    float Metallic;
    float2 TilingFactor;
};

StructuredBuffer<PerObjectData> g_InputPerObjectDatas : register(t0, space1);
ByteAddressBuffer AllPerMaterialData : register(t1, space1);
StructuredBuffer<uint> g_VisibleInstanceIndices: register(t2, space1);

PerMaterialData LoadPerMaterialData(uint index)
{
    PerMaterialData data;

    uint4 diffuseColorRaw = AllPerMaterialData.Load4(index);
    uint4 specularColorRaw = AllPerMaterialData.Load4(index + 16);
    uint4 roughnessMetallicTilingRaw = AllPerMaterialData.Load4(index + 32);

    data.DiffuseColor = asfloat(diffuseColorRaw);
    data.SpecularColor = asfloat(specularColorRaw);
    float4 temp = asfloat(roughnessMetallicTilingRaw);
    data.Roughness = temp.x;
    data.Metallic = temp.y;
    data.TilingFactor = temp.zw;

    return data;
}

#endif
