#include "PreCompiledHeader.h"
#include "ComputeShader.h"
#include "Renderer/RenderAPI.h"
namespace EngineCore
{
    ComputeShader::ComputeShader(const string &path)
    {
        RenderAPI::GetInstance()->CompileComputeShader(path, this);
    }
}