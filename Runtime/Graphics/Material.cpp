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

    Material::~Material()
    {
        if (data) delete data;
        if (shader) delete shader;
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
        for (const auto& pair : data->vec3Data)
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

    }


}
