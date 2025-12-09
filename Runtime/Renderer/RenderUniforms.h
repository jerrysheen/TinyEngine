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

   struct PerObjectData
   {
      Matrix4x4 objectToWorld;
   };

   struct DrawIndices
   {
       uint32_t objectIndex;
       uint32_t materialIndex;
   };

   struct PerPassData_Shadow
   {
      float temp;
   };

};