#include "PreCompiledHeader.h"
#include "ComputeShader.h"
#include "Renderer/RenderAPI.h"
namespace EngineCore
{
    ComputeShader::ComputeShader(const string &path)
    {
        RenderAPI::GetInstance()->CompileComputeShader(path, this);
    }

    void ComputeShader::SetBuffer(const std::string &name, IGPUBuffer* buffer)
    {
        bool find = false;
        for(auto& bindingInfo : mShaderReflectionInfo.mUavInfo)
        {
            if(bindingInfo.resourceName == name)
            {
                find = true;
            }
        }
        for(auto& bindingInfo : mShaderReflectionInfo.mTextureInfo)
        {
            if(bindingInfo.resourceName == name)
            {
                find = true;
            }
        }
        for(auto& bindingInfo : mShaderReflectionInfo.mConstantBufferInfo)
        {
            if(bindingInfo.resourceName == name)
            {
                find = true;
            }
        }

        ASSERT(find == true);
        resourceMap[name] = buffer;
    }
}