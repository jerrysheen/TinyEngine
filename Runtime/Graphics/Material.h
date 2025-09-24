#pragma once
#include "Math.h"
#include "MaterialData.h"
#include "Core/Resources.h"
#include "Shader.h"

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
    public:
        static int ID;
    private:
        int matID;
        void InitMaterialData(const MaterialStruct* matStruct);
    };
}