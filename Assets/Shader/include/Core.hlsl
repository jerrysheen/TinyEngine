#ifndef CORE_HLSLI
#define CORE_HLSLI


cbuffer PerFrameData : register(b0, space0)
{
    float3 LightDirection;
    float LightIntensity;
    float3 LightColor;
    float padding;
    float3 AmbientColor;
    float AmbientStrength;
    float Time;
}

cbuffer PerPassData : register(b0, space1)
{
    float3 CameraPosition;
    float4x4 ViewMatrix; 
    float4x4 ProjectionMatrix;
}

cbuffer DrawIndices : register(b0, space2)
{
    uint objectIndex;
}


struct PerObjectData
{
    float4x4 objectToWorld;
    // 4byte
    uint matIndex;
    float3 padding;
};

#endif
