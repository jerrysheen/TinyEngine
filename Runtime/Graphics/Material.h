#pragma once
#include "Math.h"
#include "MaterialData.h"
#include "Core/Resources.h"
#include "Shader.h"
#include "Texture.h"

namespace EngineCore
{
    class Material
    {
    public:
        MaterialData* data;
        Shader* shader;
        Material(const MaterialStruct* matStruct);
        Material();
        void SetUpGPUResources();
        void UploadDataToGpu();
        inline int GetID() const {return matID;};
        ~Material();

        void SetTexture(const string& name, const Texture& texture);
        void SetFloat(const string& name, float value);
    public:
        static int ID;
    private:
        int matID;
        void InitMaterialData(const MaterialStruct* matStruct);
    };
}