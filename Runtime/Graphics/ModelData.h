#pragma once
#include "PreCompiledHeader.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Core/Resources.h"
#include "Core/PublicStruct.h"

namespace EngineCore
{

    class ModelData
    {
    public:
        uint32_t VAO;
        std::vector<Vertex> vertex;
        std::vector<int> index;
        std::vector<InputLayout> layout;
    };

}