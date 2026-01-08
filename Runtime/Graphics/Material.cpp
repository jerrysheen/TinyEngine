#include "PreCompiledHeader.h"
#include "Material.h"
#include "Renderer/RenderAPI.h"
#include "Resources/ResourceManager.h"
#include "Resources/Asset.h"
#include "Graphics/GPUSceneManager.h"
#include "Renderer/RenderStruct.h"

namespace EngineCore
{
    Material::Material(MetaData* metaData) : Resource(metaData)
    {
        mAssetType = AssetType::Material;

        MaterialMetaData* matMetaData = static_cast<MaterialMetaData*>(metaData);
        auto& dependenciesMap = metaData->dependentMap;
        LoadDependency(dependenciesMap);

        ASSERT(mShader.IsValid());
        SetUpRenderState();
        GetTextureInfoFromShaderReflection();
        MaterialLayout layout = MaterialLayout::GetDefaultPBRLayout();
        matInstance = std::make_unique<MaterialInstance>(layout);
        SetUpRenderState();
        materialAllocation = GPUSceneManager::GetInstance()->GetSinglePerMaterialData();
        UploadDataToGpu();

    }

    Material::Material(ResourceHandle<Shader> shader)
        : mShader(shader)
    {
        mAssetType = AssetType::Material;

        GetTextureInfoFromShaderReflection();
        MaterialLayout layout = MaterialLayout::GetDefaultPBRLayout();
        matInstance = std::make_unique<MaterialInstance>(layout);

        SetUpRenderState();
        materialAllocation = GPUSceneManager::GetInstance()->GetSinglePerMaterialData();
        UploadDataToGpu();
    }

    Material::Material(const Material &other)
        : mShader(other.mShader),
        mRenderState(other.mRenderState)
    {
        mAssetType = AssetType::Material;

        GetTextureInfoFromShaderReflection();
        MaterialLayout layout = other.matInstance->GetLayout();
        matInstance = std::make_unique<MaterialInstance>(layout);
        SetUpRenderState();
        materialAllocation = GPUSceneManager::GetInstance()->GetSinglePerMaterialData();
        UploadDataToGpu();
    }

    Material::~Material()
    {
        
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
                textureHandleMap[name] = tex;
            default:
                break;
            }
        }
    }

    void Material::SetUpRenderState()
    {
        mRenderState.shaderInstanceID = mShader.Get()->GetInstanceID();
        mRenderState.rootSignatureKey = mShader.Get()->mShaderReflectionInfo.mRootSigKey;

        //todo： 很多还没做
    }

    void Material::GetTextureInfoFromShaderReflection()
    {
        // 新增：根据 Shader 反射信息初始化 MaterialData
        ASSERT(mShader.IsValid());

        // 初始化纹理槽位（从 mTextureInfo 中获取）
        for (const auto& texInfo : mShader.Get()->mShaderReflectionInfo.mTextureInfo)
        {
            // 如果有，说明是在LoadDependency的时候就加载进来了
            if (textureHandleMap.count(texInfo.resourceName) > 0) 
            {
                textureData[texInfo.resourceName] = textureHandleMap[texInfo.resourceName].Get()->textureBuffer;
            }
            else 
            {
                textureData[texInfo.resourceName] = nullptr;  // 初始化为空
            }
        }
    }

    // 根据Shader信息，创建GPU buffer，进行数据映射。

    
    
    void Material::UploadDataToGpu()
    {
        if (matInstance)
        {
            GPUSceneManager::GetInstance()->allMaterialDataBuffer
                ->UploadBuffer(materialAllocation, matInstance->GetData(), matInstance->GetSize());
        }
        
    }


}
