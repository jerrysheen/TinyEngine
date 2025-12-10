#include "PreCompiledHeader.h"
#include "Material.h"
#include "Renderer/RenderAPI.h"
#include "Resources/ResourceManager.h"
#include "Resources/Asset.h"
#include "Graphics/GPUSceneManager.h"

namespace EngineCore
{
    Material::Material(MetaData* metaData) : Resource(metaData)
    {
        mAssetType = AssetType::Material;

        MaterialMetaData* matMetaData = static_cast<MaterialMetaData*>(metaData);
        // 加载引用
        for (const auto& [key, value] : matMetaData->textureData)
        {
            textureData[key] = value;
        }
        auto& dependenciesMap = metaData->dependentMap;
        LoadDependency(dependenciesMap);

        ASSERT(mShader.IsValid());
        SetUpRenderState();

        SetUpGPUResources();

        MaterialLayout layout = MaterialLayout::GetDefaultPBRLayout();
        Vector4 color = {1,1,1,0};
        matInstance = std::make_unique<MaterialInstance>(layout);
        SetValue("DiffuseColor", &color, sizeof(Vector4));
        materialAllocation = GPUSceneManager::GetInstance()->GetSinglePerMaterialData();

        UploadDataToGpu();

    }

    Material::Material(ResourceHandle<Shader> shader)
        : mShader(shader)
    {
        MaterialLayout layout = MaterialLayout::GetDefaultPBRLayout();
        matInstance = std::make_unique<MaterialInstance>(layout);

        SetUpRenderState();
        SetUpGPUResources();
        materialAllocation = GPUSceneManager::GetInstance()->GetSinglePerMaterialData();
        UploadDataToGpu();
    }

    Material::Material(const Material &other)
        : mShader(other.mShader),
        mTexResourceMap(other.mTexResourceMap), mRenderState(other.mRenderState)
    {
        mAssetType = AssetType::Material;
        // 创建GPU资源,并且进行同步.
        SetUpGPUResources();
        materialAllocation = GPUSceneManager::GetInstance()->GetSinglePerMaterialData();
    }

    Material::~Material()
    {
        
    }

    void Material::SetTexture(const string& slotName, ResourceHandle<Texture> handle)
    {
        //cpu, 更新data数据，
        ASSERT(handle.IsValid());
        textureData[slotName] = handle.Get();
        int slotIndex = -1;
        for (auto& textureInfo : mShader.Get()->mShaderReflectionInfo.mTextureInfo)
        {
            if (textureInfo.resourceName == slotName) 
            {
                slotIndex = textureInfo.registerSlot;
            }
        }
        ASSERT_MSG(slotIndex != -1, "Can't find this Texture");
        //ASSERT(mTexResourceMap.count(slotName) > 0);
        // 保存新的，释放原来的
        mTexResourceMap[slotName] = handle;
        //gpu
        RenderAPI::GetInstance()->SetShaderTexture(this, slotName ,slotIndex, handle.Get()->GetInstanceID());
    }
    
    void Material::SetTexture(const string& slotName, ResourceHandle<FrameBufferObject> handle)
    {
        //cpu, 更新data数据，
        ASSERT(handle.IsValid());
        
        textureData[slotName] = handle.Get();
        int slotIndex = -1;
        for (auto& textureInfo : mShader.Get()->mShaderReflectionInfo.mTextureInfo)
        {
            if (textureInfo.resourceName == slotName) 
            {
                slotIndex = textureInfo.registerSlot;
            }
        }
        ASSERT_MSG(slotIndex != -1, "Can't find this Texture");
        ASSERT(mTexResourceMap.count(slotName) > 0);
        // 保存新的，释放原来的
        mTexResourceMap[slotName] = handle;
        //gpu
        RenderAPI::GetInstance()->SetShaderTexture(this, slotName ,slotIndex, handle.Get()->GetInstanceID());
    }

     // todo：资源的统一管理， Material只会持有一个textureID，后续的信息都去ResouceManager的Texture中去找。
     void Material::SetTexture(const string& slotName, uint64_t texInstanceID)
     {
         //cpu, 更新data数据，
         int slotIndex = -1;
         for (auto& textureInfo : mShader.Get()->mShaderReflectionInfo.mTextureInfo)
         {
             if (textureInfo.resourceName == slotName)
             {
                 slotIndex = textureInfo.registerSlot;
             }
         }
         ASSERT_MSG(slotIndex != -1, "Can't find this Texture");
         //gpu
         RenderAPI::GetInstance()->SetShaderTexture(this, slotName, slotIndex, texInstanceID);
     }


    // 1. 加载ResouceHandle资源
    // 2. 同步具体的资源到textureData中.
    void Material::LoadDependency(const std::unordered_map<std::string, MetaData> &dependentMap)
    {
        for(const auto& [name, metaData] : dependentMap )
        {
            ResourceHandle<Texture> tex;
            switch (metaData.assetType)
            {
            case AssetType::Shader:
                mShader =  ResourceManager::GetInstance()->LoadAsset<Shader>(metaData.path);
                break;
            case AssetType::Texture2D:
                tex = ResourceManager::GetInstance()->LoadAsset<Texture>(metaData.path);
                mTexResourceMap.try_emplace(name, tex);
                // 这个地方从meta加载进来, 一定是一对一,所以不考虑重复怎么处理.
                if(textureData.count(name) > 0)
                {
                    textureData[name] = tex.Get();
                }
            default:
                break;
            }
        }
    }

    void Material::SetUpRenderState()
    {
        mRenderState.shaderInstanceID = mShader.Get()->GetInstanceID();
    }

    void Material::GetMaterialDataFromShaderReflection()
    {
        // 新增：根据 Shader 反射信息初始化 MaterialData
        ASSERT(mShader.IsValid());

        // 初始化纹理槽位（从 mTextureInfo 中获取）
        for (const auto& texInfo : mShader.Get()->mShaderReflectionInfo.mTextureInfo)
        {
            textureData[texInfo.resourceName] = nullptr;  // 初始化为空
        }
    }

    // 根据Shader信息，创建GPU buffer，进行数据映射。
    void Material::SetUpGPUResources()
    {  
        RenderAPI::GetInstance()->CreateMaterialTextureSlots(this, mShader.Get()->mShaderReflectionInfo.mTextureInfo);
    }
    
    
    void Material::UploadDataToGpu()
    {
        if (matInstance)
        {
            GPUSceneManager::GetInstance()->allMaterialDataBuffer
                ->UploadBuffer(materialAllocation, matInstance->GetData(), matInstance->GetSize());
        }
        

        // todo: 这个地方设计思路有点问题，但是先不管了
        for(const auto& [name, texptr] : textureData)
        {
           if(texptr != nullptr)
           {
               SetTexture(name, texptr->GetInstanceID());
           }
        }
    }


}
