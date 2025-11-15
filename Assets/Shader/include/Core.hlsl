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

}

#endif
