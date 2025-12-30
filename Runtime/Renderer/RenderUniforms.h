#pragma once
#include "Math/Math.h"

namespace EngineCore
{
   enum class UniformBufferType : uint32_t
   {
      PerFrameData = 1,
      PerPassData_Shadow = 2,
      PerPassData_Foraward = 2,
   };

   struct PerFrameData
   {
      Vector3 LightDirection;
      float LightIntensity;
      Vector3 LightColor;
      float padding;
      Vector3 AmbientColor;
      float AmbientStrength;
      float Time;
      PerFrameData() = default;
   };

   struct PerPassData_Forward
   {
        Vector4 cameraPos;
        Matrix4x4 ViewMatrix = Matrix4x4::Identity;
        Matrix4x4 ProjectionMatrix = Matrix4x4::Identity;
   };

   struct alignas(16) PerObjectData
   {
       Matrix4x4 objectToWorld = Matrix4x4::Identity;
       uint32_t matIndex = 0;
       uint32_t renderProxyStartIndex = 0;
       uint32_t renderProxyCount = 0;
       uint32_t padding[1]; // 显式填充 12 字节，确保 C++ (80字节) 与 HLSL 布局严格一致
       PerObjectData() = default;
   };

   struct PerPassData_Shadow
   {
      float temp;
   };

   
};