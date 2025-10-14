#pragma once
#include "Math.h"
#include "MaterialData.h"
#include "Core/Resources.h"
#include "Shader.h"
#include "Texture.h"
#include "Core/Object.h"
#include "Core/PublicStruct.h"

namespace EngineCore
{
    class Material : public Object
    {
    public:
        MaterialData* data;
        Shader* shader;
        Material(const MaterialStruct* matStruct);
        Material();
        void SetUpGPUResources();
        void UploadDataToGpu();
        ~Material();

        void SetTexture(const string& name, const Texture* texture);
        void SetTexture(const string& name, uint64_t texInstanceID);
        void SetFloat(const string& name, float value);
    private:
        void InitMaterialData(const MaterialStruct* matStruct);
    };
}