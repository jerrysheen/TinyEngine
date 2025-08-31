#include "PreCompiledHeader.h"
#include "RenderAPI.h"
#include "Platforms/D3D12/D3D12RenderAPI.h"

namespace EngineCore
{
    std::unique_ptr<RenderAPI> RenderAPI::s_Instance = nullptr;
    void RenderAPI::Create()
    {
        s_Instance = std::make_unique<D3D12RenderAPI>();
    }

}