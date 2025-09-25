#pragma once
#include "PreCompiledHeader.h"
#include "Math/Math.h"
#include "PublicEnum.h"
#include "Graphics/Texture.h"
// 添加D3D12相关头文件
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl.h>

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
        unordered_map<string, Matrix4x4> matrix4x4Data;
        unordered_map<string, Texture> textureData;
    };




    class Resources
    {
    public:
        static MaterialStruct* LoadMaterial(const std::string& path);
        static ShaderVariableType GetShaderVaribleType(uint32_t size);
    };

}