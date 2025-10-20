#pragma once
#include "Math.h"
#include "MaterialData.h"
#include "Resources/MetaFile.h"
#include "Shader.h"
#include "Texture.h"
#include "Core/Object.h"
#include "Core/PublicStruct.h"
#include "Renderer/RenderCommand.h"
#include "Resources/ResourceHandle.h"

namespace EngineCore
{
    class Material : public Resource
    {
    public:
        MaterialData mMaterialdata;

        // 真正持有的资源句柄， 别的都是资源的ID等，
        ResourceHandle<Shader> mShader;
        unordered_map<std::string, ResourceHandle<Texture>> mTexResourceMap;
        Material() = default;
        Material(MetaData* metaData);
        Material(ResourceHandle<Shader> shader);
        void SetUpGPUResources();
        void UploadDataToGpu();
        ~Material();

        void SetTexture(const string& name, Texture* texture);
        //void SetTexture(const string& name, const ResourceHandle<Texture> texHandle);
        void SetTexture(const string& name, uint64_t texInstanceID);
        void SetFloat(const string& name, float value);

        inline MaterailRenderState GetMaterialRenderState() const { return renderState;};
        // todo  自己同步材质；
        MaterailRenderState renderState;
    private:
        void LoadDependency(const std::unordered_map<std::string, MetaData>& dependentMap);
        
    };
}