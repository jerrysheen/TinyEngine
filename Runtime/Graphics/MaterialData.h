#pragma once
#include "PreCompiledHeader.h"
#include "Math/Math.h"
#include "Graphics/ResourceStruct.h"

namespace EngineCore
{
    class MaterialData
    {
    public:
        unordered_map<string, float> floatData;
        unordered_map<string, Vector2> vec2Data;
        unordered_map<string, Vector3> vec3Data;

        
        MaterialData& operator=(const MaterialStruct& other) {  // 拷贝赋值操作符
            floatData = other.floatData;
            vec2Data = other.vec2Data;
            vec3Data = other.vec3Data;
            return *this;
        } 
    };
}