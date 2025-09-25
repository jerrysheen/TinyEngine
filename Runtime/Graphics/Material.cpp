#include "PreCompiledHeader.h"
#include "Material.h"
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    int Material::ID = 0;

    Material::Material(const MaterialStruct* matStruct)
    {
        matID = ID;
        ID++;
        data = new MaterialData();  // 分配内存
        shader = nullptr;           // 初始化为nullptr
        InitMaterialData(matStruct);
    }

    Material::Material() 
    {
        matID = ID;
        ID++;
        data = new MaterialData();
        shader = nullptr;
        // 空初始化表示不需要
        // 
        // InitMaterialData(matStruct);
    }

    Material::~Material()
    {
        if (data) delete data;
        if (shader) delete shader;
    }

    // 同步这个数据到CPU和GPU
    void Material::SetTexture(const string &slotName, const Texture &texture)
    {
        //cpu, 更新data数据，
        data->textureData[slotName] = texture;
        int slotIndex = -1;
        for (auto& textureInfo : shader->mShaderBindingInfo->mTextureInfo) 
        {
            if (textureInfo.resourceName == slotName) 
            {
                slotIndex = textureInfo.registerSlot;
            }
        }
        ASSERT_MSG(slotIndex != -1, "Can't find this Texture");
        //gpu
        RenderAPI::GetInstance().SetShaderTexture(this, slotName ,slotIndex,texture);
    }

    void Material::SetFloat(const string &name, float value)
    {
        data->floatData[name] = value;
        auto& map = shader->mShaderBindingInfo->mShaderStageVariableInfoMap;
        auto& variableInfo = map[name];
        RenderAPI::GetInstance().SetShaderFloat(this, variableInfo, value);
    }

    // 根据Shader信息，创建GPU buffer，进行数据映射。
    // todo： 数据优化，第一版设计，会在VS PS上传重复数据。
    // CPU handle在哪里持有？
    // 后续资源应该用inFo.name 进行寻找
    void Material::SetUpGPUResources()
    {  
        RenderAPI::GetInstance().CreateBuffersResource(this, shader->mShaderBindingInfo->mBufferInfo);
        RenderAPI::GetInstance().CreateSamplerResource(this, shader->mShaderBindingInfo->mSamplerInfo);
        RenderAPI::GetInstance().CreateTextureResource(this, shader->mShaderBindingInfo->mTextureInfo);
        RenderAPI::GetInstance().CreateUAVResource(this, shader->mShaderBindingInfo->mUavInfo);


        UploadDataToGpu();
    }
    
    void Material::InitMaterialData(const MaterialStruct* matStruct)
    {
        *data = *matStruct;
    }
    
    
    void Material::UploadDataToGpu()
    {
        // 设置数据部分，
        // 目前先设置ps好了， 理论上来说ps vs的数据应该是如果有一样的就设置一次就好了
        // 方法1: 使用 C++11 range-based for loop (推荐)
        for(const auto& pair : data->vec3Data)
        {
            const string& key = pair.first;     
            const Vector3& value = pair.second;
            
            auto& map = shader->mShaderBindingInfo->mShaderStageVariableInfoMap;
            if(map.count(key) > 0)
            {
                auto& variableInfo = map[key];
                RenderAPI::GetInstance().SetShaderVector(this, variableInfo, value);
            }
        }
        
        for(const auto& pair : data->matrix4x4Data)
        {
            const string& key = pair.first;
            const Matrix4x4& value = pair.second;
            auto& map = shader->mShaderBindingInfo->mShaderStageVariableInfoMap;
            if(map.count(key) > 0)
            {
                auto& variableInfo = map[key];
                RenderAPI::GetInstance().SetShaderMatrix4x4(this, variableInfo, value);
            }

        }


    }


}
