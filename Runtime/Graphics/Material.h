#pragma once
#include "Math.h"
#include "Serialization/MetaData.h"
#include "Shader.h"
#include "Buffer.h"
#include "Core/Object.h"
#include "Core/PublicStruct.h"
#include "Renderer/RenderCommand.h"
#include "Resources/ResourceHandle.h"
#include <variant>
#include "Graphics/FrameBufferObject.h"
#include "Graphics/Texture.h"
#include "Renderer/RenderStruct.h"
#include "MaterialInstance.h"

namespace EngineCore
{
    class Material : public Resource
    {
    public:
        bool isDirty = true;
        std::unique_ptr<MaterialInstance> matInstance;
        unordered_map<string, Buffer2D*> textureData;
        ResourceHandle<Shader> mShader;
        // temp 后续改
        unordered_map<std::string, std::variant<ResourceHandle<Texture>, ResourceHandle<FrameBufferObject>>> mTexResourceMap;


        Material() = default;
        Material(MetaData* metaData);
        Material(ResourceHandle<Shader> shader);
        Material(const Material& other);
        void SetUpGPUResources();
        void UploadDataToGpu();
        ~Material();

        void SetTexture(const string& name, uint64_t texInstanceID);
        void SetTexture(const string& name, ResourceHandle<Texture> handle);
        void SetTexture(const string& name, ResourceHandle<FrameBufferObject> handle);

        void SetValue(const string& name, void* data, uint32_t size) 
        {
            ASSERT(matInstance != nullptr);
            matInstance->SetValue(name, data, size);
        }

        inline MaterailRenderState GetMaterialRenderState() const { return mRenderState;};
        // todo  自己同步材质；
        MaterailRenderState mRenderState;
        BufferAllocation materialAllocation;
    private:
        void LoadDependency(const std::unordered_map<std::string, MetaData>& dependentMap);
        void SetUpRenderState();
        void GetMaterialDataFromShaderReflection();
    };
}