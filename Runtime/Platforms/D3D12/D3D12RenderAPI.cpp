#include "PreCompiledHeader.h"
#include "D3D12RenderAPI.h"

namespace EngineCore
{
    void D3D12RenderAPI::BeginFrame()
    {
        std::cout << "beginFrame" << std::endl;
    }

    void D3D12RenderAPI::Render()
    {

    }

    void D3D12RenderAPI::EndFrame()
    {
        std::cout << "EndFrame" << std::endl;
    }
    
} // namespace EngineCore
