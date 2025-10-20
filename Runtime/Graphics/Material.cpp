#include "PreCompiledHeader.h"
#include "Material.h"
#include "Renderer/RenderAPI.h"
#include "Resources/ResourceManager.h"
namespace EngineCore
{
    Material::Material(MetaData* metaData) : Resource(metaData)
    {
        MaterialMetaData* matMetaData = static_cast<MaterialMetaData*>(metaData);
        // 加载引用

        // 同步MaterialData：
        mMaterialdata.floatData = matMetaData->floatData;
        mMaterialdata.vec2Data = matMetaData->vec2Data;
        mMaterialdata.vec3Data = matMetaData->vec3Data;
        mMaterialdata.matrix4x4Data = matMetaData->matrix4x4Data;
        mMaterialdata.textureData = matMetaData->textureData;

        auto& dependenciesMap = metaData->dependentMap;
        LoadDependency(dependenciesMap);

        ASSERT(mShader.IsValid());
        // 创建GPU资源,并且进行同步.
        SetUpGPUResources();
    }

    Material::Material(ResourceHandle<Shader> shader)
        : mShader(shader)
    {
        SetUpGPUResources();
    }


    Material::~Material()
    {
    }

    // 同步这个数据到CPU和GPU
    // 注意!! 这个接口只给非ResouceHandle使用
    // todo: 其实我应该统一两个接口的, 似乎是可以统一的..
    void Material::SetTexture(const string &slotName, Texture* texture)
    {
        //cpu, 更新data数据，
        ASSERT(texture != nullptr);
        mMaterialdata.textureData[slotName] = texture;
        int slotIndex = -1;
        for (auto& textureInfo : mShader.Get()->mShaderBindingInfo.mTextureInfo)
        {
            if (textureInfo.resourceName == slotName) 
            {
                slotIndex = textureInfo.registerSlot;
            }
        }
        ASSERT_MSG(slotIndex != -1, "Can't find this Texture");
        //gpu
        RenderAPI::GetInstance().SetShaderTexture(this, slotName ,slotIndex, texture->GetInstanceID());
    }

    // todo：资源的统一管理， Material只会持有一个textureID，后续的信息都去ResouceManager的Texture中去找。
    void Material::SetTexture(const string& slotName, uint64_t texInstanceID)
    {
        //cpu, 更新data数据，
        int slotIndex = -1;
        for (auto& textureInfo : mShader.Get()->mShaderBindingInfo.mTextureInfo)
        {
            if (textureInfo.resourceName == slotName)
            {
                slotIndex = textureInfo.registerSlot;
            }
        }
        ASSERT_MSG(slotIndex != -1, "Can't find this Texture");
        //gpu
        RenderAPI::GetInstance().SetShaderTexture(this, slotName, slotIndex, texInstanceID);
    }


    void Material::SetFloat(const string &name, float value)
    {
        mMaterialdata.floatData[name] = value;
        auto& map = mShader.Get()->mShaderBindingInfo.mShaderStageVariableInfoMap;
        auto& variableInfo = map[name];
        RenderAPI::GetInstance().SetShaderFloat(this, variableInfo, value);
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
                if(mMaterialdata.textureData.count(name) > 0)
                {
                    mMaterialdata.textureData[name] = tex.Get();
                }
            default:
                break;
            }
        }
    }

    // 根据Shader信息，创建GPU buffer，进行数据映射。
    // todo： 数据优化，第一版设计，会在VS PS上传重复数据。
    // CPU handle在哪里持有？
    // 后续资源应该用inFo.name 进行寻找
    void Material::SetUpGPUResources()
    {  
        RenderAPI::GetInstance().CreateBuffersResource(this, mShader.Get()->mShaderBindingInfo.mBufferInfo);
        RenderAPI::GetInstance().CreateSamplerResource(this, mShader.Get()->mShaderBindingInfo.mSamplerInfo);
        RenderAPI::GetInstance().CreateTextureResource(this, mShader.Get()->mShaderBindingInfo.mTextureInfo);
        RenderAPI::GetInstance().CreateUAVResource(this, mShader.Get()->mShaderBindingInfo.mUavInfo);


        UploadDataToGpu();
    }
    
    
    
    void Material::UploadDataToGpu()
    {
        // 设置数据部分，
        // 目前先设置ps好了， 理论上来说ps vs的数据应该是如果有一样的就设置一次就好了
        // 方法1: 使用 C++11 range-based for loop (推荐)
        for(const auto& pair : mMaterialdata.vec3Data)
        {
            const string& key = pair.first;     
            const Vector3& value = pair.second;
            
            auto& map = mShader.Get()->mShaderBindingInfo.mShaderStageVariableInfoMap;
            if(map.count(key) > 0)
            {
                auto& variableInfo = map[key];
                RenderAPI::GetInstance().SetShaderVector(this, variableInfo, value);
            }
        }
        
        for(const auto& pair : mMaterialdata.matrix4x4Data)
        {
            const string& key = pair.first;
            const Matrix4x4& value = pair.second;
            auto& map = mShader.Get()->mShaderBindingInfo.mShaderStageVariableInfoMap;
            if(map.count(key) > 0)
            {
                auto& variableInfo = map[key];
                RenderAPI::GetInstance().SetShaderMatrix4x4(this, variableInfo, value);
            }
        }

        //for(const auto& [name, texptr] : mMaterialdata.textureData)
        //{
        //    if(texptr != nullptr)
        //    {
        //        SetTexture(name, texptr);
        //    }
        //}
    }


}
