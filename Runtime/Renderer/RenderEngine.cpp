#include "PreCompiledHeader.h"
#include "RenderEngine.h"
#include "RenderAPI.h"
#include "Managers/WindowManager.h"
#include "Camera.h"

#include "Platforms/D3D12/D3D12RenderAPI.h"
#include "Managers/SceneManager.h"

namespace EngineCore
{
    std::unique_ptr<RenderEngine> RenderEngine::s_Instance = nullptr;
    
    void RenderEngine::Create()
    {
        s_Instance = std::make_unique<RenderEngine>();
        WindowManager::Create();
        RenderAPI::Create();
    }

    void RenderEngine::Update()
    {
    }    
    
    void RenderEngine::BeginRender()
    {
        RenderAPI::GetInstance().BeginFrame();
    }    
    
    void RenderEngine::OnResize(int width, int height)
    {
        RenderAPI::GetInstance().OnResize(width, height);
    }

    void RenderEngine::EndRender()
    {
        RenderAPI::GetInstance().EndFrame();
    }    
    
    void RenderEngine::Render()
    {
        //RenderAPI::GetInstance().Render();
        Camera* cam = Camera::GetMainCamera();
        auto d3d12RenderAPI = static_cast<D3D12RenderAPI*>(&RenderAPI::GetInstance());
        auto mesh = SceneManager::GetInstance().testMesh;
        auto mat = SceneManager::GetInstance().testMat;
        
        TD3D12DrawRecord record(mat, mesh);
        d3d12RenderAPI->TestRenderObj(record);
    }
}
