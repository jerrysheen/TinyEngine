#pragma once
#include "Math.h"
#include "Shader.h"
#include "Core/Object.h"
#include "Core/PublicStruct.h"
#include "Renderer/RenderCommand.h"
#include "Resources/ResourceHandle.h"
#include <variant>
#include "Graphics/Texture.h"
#include "MaterialLibrary/MaterialInstance.h"
//#include "MaterialData.h"

namespace EngineCore
{
    enum class AlphaMode : uint8_t
    {
        Opaque = 0,
        Mask = 1,
        Blend = 2
    };

    class Material : public Resource
    {
    public:
        bool isDirty = true;
        bool isBindLessMaterial = false;
        AlphaMode alphaMode = AlphaMode::Opaque;
        float alphaCutoff = 0.5f;
        float transmissionFactor = 0.0f;
        string archyTypeName = "";
        std::unique_ptr<MaterialInstance> matInstance;
        ResourceHandle<Shader> mShader;
        unordered_map<string, IGPUTexture*> textureData;
        unordered_map<std::string, ResourceHandle<Texture>> textureHandleMap;
        MaterailRenderState mRenderState;
        BufferAllocation materialAllocation;

        Material() = default;
        Material(ResourceHandle<Shader> shader);
        Material(const Material& other);
        void UploadDataToGpu();
        ~Material();


        void SetValue(const string& name, void* data, uint32_t size) 
        {
            ASSERT(matInstance != nullptr);
            matInstance->SetValue(name, data, size);
        }

        // 通用设置材质texture的接口
        void SetTexture(const string& name, IGPUTexture* texture)
        {
            ASSERT(textureData.count(name) > 0);
            if(textureHandleMap.count(name))
            {
                textureHandleMap.erase(name);
            }
            textureData[name] = texture;
        }

        // 运行时关联一个临时资源，建立一个引用， 防止资源因为0引用被销毁
        void SetTexture(const string& name, ResourceHandle<Texture> texture)
        {
            ASSERT(textureData.count(name) > 0);
            textureHandleMap[name] = texture;
            if(texture.IsValid())
            {
                textureData[name] = texture.Get()->textureBuffer;
            }
        }

        // only for serialization
        void SetTexture(const string& name, uint64_t asset)
        {
            ResourceHandle<Texture> texHandle;
            texHandle.mAssetID = AssetID(asset);
            textureHandleMap[name] = texHandle;
        }
        inline MaterailRenderState GetMaterialRenderState() const { return mRenderState; };
        
        virtual void OnLoadComplete() override;
    private:
        void SetUpRenderState();
        void GetTextureInfoFromShaderReflection();
    };
}
