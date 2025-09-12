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
        unordered_map<string, Vector2> vec2Data;
        unordered_map<string, Vector3> vec3Data;

    };

    class Resources
    {
    public:
        static MaterialStruct* LoadMaterial(const std::string& path);
    };

}