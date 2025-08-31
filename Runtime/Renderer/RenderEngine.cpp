#include "PreCompiledHeader.h"
#include "RenderEngine.h"
#include "RenderAPI.h"

namespace EngineCore
{
    std::unique_ptr<RenderEngine> RenderEngine::s_Instance = nullptr;
    
    void RenderEngine::Create()
    {
        s_Instance = std::make_unique<RenderEngine>();
        RenderAPI::Create();
    }

    void RenderEngine::Update()
    {

    }    
    
    void RenderEngine::BeginRender()
    {
        RenderAPI::GetInstance().BeginFrame();
    }    
    
    void RenderEngine::EndRender()
    {

    }    
    
    void RenderEngine::Render()
    {

    }
}
