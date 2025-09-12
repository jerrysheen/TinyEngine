#pragma once
#include "Math.h"
#include "MaterialData.h"
#include "Graphics/ResourceStruct.h"

namespace EngineCore
{
    class Material
    {
    public:
        MaterialData* data;
        Material(const MaterialStruct* matStruct);
    private:
        void InitMaterialData(const MaterialStruct* matStruct);
    };


}