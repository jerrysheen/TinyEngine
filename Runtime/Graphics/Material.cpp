#include "PreCompiledHeader.h"
#include "Material.h"
#include "Renderer/RenderAPI.h"
#include "Resources/ResourceManager.h"
#include "Resources/Asset.h"

namespace EngineCore
{
    Material::Material(MetaData* metaData) : Resource(metaData)
    {
        mAssetType = AssetType::Material;

        MaterialMetaData* matMetaData = static_cast<MaterialMetaData*>(metaData);
        // 加载引用

        // 同步MaterialData：
        mMaterialdata.floatData = matMetaData->floatData;
        mMaterialdata.vec2Data = matMetaData->vec2Data;
        mMaterialdata.vec3Data = matMetaData->vec3Data;
        mMaterialdata.matrix4x4Data = matMetaData->matrix4x4Data;
        for (const auto& [key, value] : matMetaData->textureData) 
        {
            mMaterialdata.textureData[key] = value;
        }

        auto& dependenciesMap = metaData->dependentMap;
        LoadDependency(dependenciesMap);

        ASSERT(mShader.IsValid());
        SetUpRenderState();

        // 创建GPU资源,并且进行同步.
        SetUpGPUResources();
    }

    Material::Material(ResourceHandle<Shader> shader)
        : mShader(shader)
    {
        GetMaterialDataFromShaderReflection();
        SetUpRenderState();
        SetUpGPUResources();
    }

    Material::Material(const Material &other)
        : mMaterialdata(other.mMaterialdata), mShader(other.mShader),
        mTexResourceMap(other.mTexResourceMap), mRenderState(other.mRenderState)
    {
        mAssetType = AssetType::Material;

        // 创建GPU资源,并且进行同步.
        SetUpGPUResources();
    }

    Material::~Material()
    {
    }

    void Material::SetTexture(const string& slotName, ResourceHandle<Texture> handle)
    {
        //cpu, 更新data数据，
        ASSERT(handle.IsValid());
        mMaterialdata.textureData[slotName] = handle.Get();
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
        
        mMaterialdata.textureData[slotName] = handle.Get();
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

    void Material::SetMatrix4x4(const string &name, const Matrix4x4 &matrix4x4)
    {
        ASSERT(mMaterialdata.matrix4x4Data.count(name) > 0);
        mMaterialdata.matrix4x4Data[name] = matrix4x4;
        auto& map = mShader.Get()->mShaderReflectionInfo.mPerMaterialConstantBuffferReflectionInfo;
        ASSERT(map.count(name) > 0);
        auto& variableInfo = map[name];
        RenderAPI::GetInstance()->SetShaderMatrix4x4(this, variableInfo, matrix4x4);
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


    void Material::SetFloat(const string &name, float value)
    {
        ASSERT(mMaterialdata.floatData.count(name) > 0);
        mMaterialdata.floatData[name] = value;
        auto& map = mShader.Get()->mShaderReflectionInfo.mPerMaterialConstantBuffferReflectionInfo;
        auto& variableInfo = map[name];
        RenderAPI::GetInstance()->SetShaderFloat(this, variableInfo, value);
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

    void Material::SetUpRenderState()
    {
        mRenderState.shaderInstanceID = mShader.Get()->GetInstanceID();
    }

    void Material::GetMaterialDataFromShaderReflection()
    {
        // 新增：根据 Shader 反射信息初始化 MaterialData
        ASSERT(mShader.IsValid());
        auto& reflectionInfo = mShader.Get()->mShaderReflectionInfo.mPerMaterialConstantBuffferReflectionInfo;
        // 遍历反射信息，根据类型初始化对应的数据槽位
        for (const auto& [name, constInfo] : reflectionInfo)
        {
            switch (constInfo.type)
            {
            case ShaderVariableType::FLOAT:
                mMaterialdata.floatData[name] = 0.0f;  // 默认值
                break;
            case ShaderVariableType::VECTOR2:
                mMaterialdata.vec2Data[name] = Vector2::Zero;
                break;
            case ShaderVariableType::VECTOR3:
                mMaterialdata.vec3Data[name] = Vector3::Zero;
                break;
            case ShaderVariableType::MATRIX4X4:
                mMaterialdata.matrix4x4Data[name] = Matrix4x4::Identity;
                break;
            // 如果有 VECTOR4 或其他类型，需要添加对应的处理
            default:
                // LOG_WARNING("Unsupported shader variable type for: " + name);
                break;
            }
        }
        
        // 初始化纹理槽位（从 mTextureInfo 中获取）
        for (const auto& texInfo : mShader.Get()->mShaderReflectionInfo.mTextureInfo)
        {
            mMaterialdata.textureData[texInfo.resourceName] = nullptr;  // 初始化为空
        }
    }

    // 根据Shader信息，创建GPU buffer，进行数据映射。
    // todo： 数据优化，第一版设计，会在VS PS上传重复数据。
    // CPU handle在哪里持有？
    // 后续资源应该用inFo.name 进行寻找
    void Material::SetUpGPUResources()
    {  
        RenderAPI::GetInstance()->CreateMaterialConstantBuffers(this, mShader.Get()->mShaderReflectionInfo.perMaterialBufferSize);
        RenderAPI::GetInstance()->CreateMaterialSamplerSlots(this, mShader.Get()->mShaderReflectionInfo.mSamplerInfo);
        RenderAPI::GetInstance()->CreateMaterialTextureSlots(this, mShader.Get()->mShaderReflectionInfo.mTextureInfo);
        RenderAPI::GetInstance()->CreateMaterialUAVSlots(this, mShader.Get()->mShaderReflectionInfo.mUavInfo);


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
            
            auto& map = mShader.Get()->mShaderReflectionInfo.mPerMaterialConstantBuffferReflectionInfo;
            if(map.count(key) > 0)
            {
                auto& variableInfo = map[key];
                RenderAPI::GetInstance()->SetShaderVector(this, variableInfo, value);
            }
        }
        
        for(const auto& pair : mMaterialdata.matrix4x4Data)
        {
            const string& key = pair.first;
            const Matrix4x4& value = pair.second;
            auto& map = mShader.Get()->mShaderReflectionInfo.mPerMaterialConstantBuffferReflectionInfo;
            if(map.count(key) > 0)
            {
                auto& variableInfo = map[key];
                RenderAPI::GetInstance()->SetShaderMatrix4x4(this, variableInfo, value);
            }
        }

        // todo: 这个地方设计思路有点问题，但是先不管了
        for(const auto& [name, texptr] : mMaterialdata.textureData)
        {
           if(texptr != nullptr)
           {
               SetTexture(name, texptr->GetInstanceID());
           }
        }
    }


}
