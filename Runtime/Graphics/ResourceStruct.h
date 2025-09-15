#pragma once
#include "PreCompiledHeader.h"
#include "Math/Math.h"

namespace EngineCore
{

    struct MaterialStruct
    {
        string name;
		string path;
		string shaderPath;       
		string shaderCode;
        unordered_map<string, float> floatData;
        unordered_map<string, Vector2> vec2Data;
        unordered_map<string, Vector3> vec3Data;

    };

    enum class VertexAttribute
    {
        POSITION,
        NORMAL,
        TANGENT,
        UV0
    };


    // 用来描述model Input 或者 shader reflection input
    // todos: shader inputlayout 部分的控制， 目前只是简单把值塞过来
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
        
        InputLayout(VertexAttribute type) : type(type) {};
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

    struct ShaderVariableInfo
    {
        string variableName;
        ShaderVariableType type;
        int bufferIndex;
        int size;
        int offset;
    };

    // 统一的 UAV、 buffer、 textureinfo、sampler
    struct ShaderResourceInfo
    {
        string resourceName;
        ShaderResourceType type;
        int registerSlot;              
        int size = 0;                  // 对CB有意义，其他资源可为0
        struct ShaderResourceInfo(const string& resourceName, ShaderResourceType type, int registerSlot, int size)
            : resourceName(resourceName), type(type), registerSlot(registerSlot), size(size)
        {};
    };

    struct ShaderStageInfo
    {
        ShaderStageType type;
        vector<ShaderResourceInfo> mTextureInfo;
        vector<ShaderResourceInfo> mSamplerInfo;
        vector<ShaderResourceInfo> mBufferInfo;
        vector<ShaderResourceInfo> mUavInfo;

        ShaderStageInfo(){};

        unordered_map<string, ShaderVariableInfo> mShaderStageVariableInfoMap;

    };


    class Resources
    {
    public:
        static MaterialStruct* LoadMaterial(const std::string& path);
        static ShaderVariableType GetShaderVaribleType(uint32_t size);
    };

}