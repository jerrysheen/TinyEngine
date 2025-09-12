#pragma once
#include "PreCompiledHeader.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"

namespace EngineCore
{
    enum class VertexAttribute
    {
        POSITION,
        NORMAL,
        TANGENT,
        UV0
    };

    struct InputLayout
    {
        VertexAttribute type;
        int size;
        int dimension;
        int stride;
        int offset;
        InputLayout(VertexAttribute _type, int _size, int _dimension, int _stride, int _offset)
        {
            type = _type; size = _size; dimension = _dimension; stride = _stride; offset = _offset;
        };
    };

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
        std::vector<InputLayout> layout;
    };

}