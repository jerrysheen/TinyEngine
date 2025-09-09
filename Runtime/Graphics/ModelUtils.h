#pragma once
#include "ModelData.h"

namespace EngineCore
{
    class ModelUtils
    {
    public:
        static ModelData* LoadMesh(const std::string& path);    

    };
}