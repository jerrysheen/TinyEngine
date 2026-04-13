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
        for(auto& bindingInfo : mShaderReflectionInfo.mBufferInfo)
        {
            if(bindingInfo.resourceName == name)
            {
                find = true;
            }
        }

        ASSERT(find == true);
        resourceMap[name] = buffer;
    }

    void ComputeShader::SetTexture(const std::string &name, IGPUTexture *texture, uint32_t mipLevel)
    {
        bool find = false;
        bool isUav = false;
        for(auto& bindingInfo : mShaderReflectionInfo.mTextureInfo)
        {
            if(bindingInfo.resourceName == name)
            {
                find = true;
                isUav = bindingInfo.type == ShaderResourceType::RWTEXTURE ? true : false;
            }
        }

        ASSERT(find == true);
        textureResourceMap[name] = {texture, isUav, mipLevel};
    }
}