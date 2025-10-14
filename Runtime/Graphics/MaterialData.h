#pragma once
#include "PreCompiledHeader.h"
#include "Math/Math.h"
#include "Core/Resources.h"
#include "Texture.h"

namespace EngineCore
{
    class MaterialData
    {
    public:
        unordered_map<string, float> floatData;
        unordered_map<string, Vector2> vec2Data;
        unordered_map<string, Vector3> vec3Data;
        unordered_map<string, Matrix4x4> matrix4x4Data;
        unordered_map<string, const Texture*> textureData;
        
        MaterialData& operator=(const MaterialStruct& other) {  // 拷贝赋值操作符
            floatData = other.floatData;
            vec2Data = other.vec2Data;
            vec3Data = other.vec3Data;
            matrix4x4Data = other.matrix4x4Data;

            // todo: texture 结构填入
            // textureData =;
            return *this;
        } 
    };
}