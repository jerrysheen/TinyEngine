#pragma once

namespace EngineCore
{
    enum class RenderDataFrenquent : UINT
    {
        PerFrameData = 0,
        PerPassData = 1,
        PerMaterialData = 2,
        PerDrawData = 3
    };

    enum class ShaderVariableType
    {
        FLOAT, VECTOR2, VECTOR3, VECTOR4, MATRIX4X4, MATRIX3X4, MATRIX3X3, UNKNOWN
    };

    enum class ShaderStageType
    {
        VERTEX_STAGE, FRAGMENT_STAGE
    };

    enum class ShaderResourceType
    {
        CONSTANT_BUFFER,
        TEXTURE,
        SAMPLER,
        UAV
    };

    enum class VertexAttribute
    {
        POSITION,
        NORMAL,
        TANGENT,
        UV0
    };

    enum class TextureDimension
    {
        TEXTURE2D,
        TEXTURE3D,
        TEXTURECUBE,
        TEXTURE2DARRAY
    };

    enum class TextureFormat
    {
        R8G8B8A8,
        D24S8,
        EMPTY,
    };

    enum class Primitive
    {
        Quad,
        Cube,
        Sphere
    };
   
    inline ShaderVariableType InferShaderVaribleTypeBySize(uint32_t size)
    {
        // 以byte为准 比如float为4byte
        switch (size)
        {
        case 4:
            return ShaderVariableType::FLOAT; 
            break;
        case 8:
            return ShaderVariableType::VECTOR2;
            break;
        case 12:
            return ShaderVariableType::VECTOR2;
            break;
        case 16:
            return ShaderVariableType::VECTOR2;
            break;
        case 64:
            return ShaderVariableType::VECTOR2;
            break;
        case 48:
            return ShaderVariableType::VECTOR2;
            break;
        case 36:
            return ShaderVariableType::VECTOR2;
            break;
        default:
            break;
        }
    }
    
    enum class SortingCriteria
    {
        ComonOpaque,
        ComonTransparent
    };
}
