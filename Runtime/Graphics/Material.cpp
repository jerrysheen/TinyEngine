#include "PreCompiledHeader.h"
#include "Material.h"
#include "Renderer/RenderAPI.h"
#include "Resources/ResourceManager.h"
#include "Resources/Asset.h"
#include "Scene/GPUSCene.h"
#include "Renderer/RenderStruct.h"
#include "MaterialLibrary/MaterialArchetypeRegistry.h"
#include "Renderer/RenderEngine.h"

namespace EngineCore
{

    Material::Material(ResourceHandle<Shader> shader)
        : mShader(shader)
    {
        mAssetType = AssetType::Material;

        GetTextureInfoFromShaderReflection();
        MaterialLayout layout = MaterialArchetypeRegistry::GetInstance().GetArchytypeLayout(mShader->name);
        if (layout.GetSize() > 0) 
        {
            matInstance = std::make_unique<MaterialInstance>(layout);
            materialAllocation = RenderEngine::GetInstance()->GetGPUScene().GetSinglePerMaterialData();
        }

        SetUpRenderState();
        UploadDataToGpu();
    }

    Material::Material(const Material &other)
        : mShader(other.mShader),
        mRenderState(other.mRenderState),
        alphaMode(other.alphaMode),
        alphaCutoff(other.alphaCutoff),
        transmissionFactor(other.transmissionFactor)
    {
        mAssetType = AssetType::Material;

        GetTextureInfoFromShaderReflection();
        MaterialLayout layout = other.matInstance->GetLayout();
        if (layout.GetSize() > 0)
        {
            matInstance = std::make_unique<MaterialInstance>(layout);
            materialAllocation = RenderEngine::GetInstance()->GetGPUScene().GetSinglePerMaterialData();
        }
        SetUpRenderState();
        UploadDataToGpu();
    }

    Material::~Material()
    {
        
    }

    void Material::OnLoadComplete()
    {
        GetTextureInfoFromShaderReflection();
        if (!matInstance) 
        {
            MaterialLayout layout = MaterialArchetypeRegistry::GetInstance().GetArchytypeLayout (archyTypeName);
            if (layout.GetSize() > 0)
            {
                matInstance = std::make_unique<MaterialInstance>(layout);
            }
        }
        materialAllocation = RenderEngine::GetInstance()->GetGPUScene().GetSinglePerMaterialData();
        SetUpRenderState();
        if(materialAllocation.isValid)UploadDataToGpu();
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
                textureData[texInfo.resourceName] = ResourceManager::GetInstance()->mDefaultTexture->textureBuffer;  // 初始化为空
            }
        }
    }

    // 根据Shader信息，创建GPU buffer，进行数据映射。

    
    
    void Material::UploadDataToGpu()
    {
        if (matInstance)
        {
            RenderEngine::GetInstance()->GetGPUScene().GetAllMaterialDataBuffer()
                ->UploadBuffer(materialAllocation, matInstance->GetInstanceData().data(), matInstance->GetSize());
        }
        
    }


}
