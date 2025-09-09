#pragma once
#include "PreCompiledHeader.h"

namespace EngineCore
{
    struct vertexLayout
    {
        int stride;
        int offset;
        int size;
    };

    class ModelData
    {
    public:
        std::vector<float> vertex;
        std::vector<int> index;
    };

}