#pragma once
#include "PreCompiledHeader.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"

namespace EngineCore
{
    struct Vertex
    {
        Vector3 position;
        Vector3 normal;
        Vector2 uv;
    };

    class ModelData
    {
    public:
        std::vector<Vertex> vertex;
        std::vector<int> index;
    };

}