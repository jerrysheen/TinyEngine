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
      float temp;
   };

   struct PerPassData_Shadow
   {
      float temp;
   };

};