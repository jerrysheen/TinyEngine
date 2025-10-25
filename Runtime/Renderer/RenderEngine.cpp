#include "PreCompiledHeader.h"
#include "RenderEngine.h"
#include "RenderAPI.h"
#include "Managers/WindowManager.h"
#include "GameObject/Camera.h"
#include "Renderer.h"
#include "RenderPipeLine/OpaqueRenderPass.h"
#include "Platforms/D3D12/D3D12RenderAPI.h"
#include "Managers/SceneManager.h"
#include "Culling.h"
#include "FrameBufferManager.h"


namespace EngineCore
{
    std::unique_ptr<RenderEngine> RenderEngine::s_Instance = nullptr;
    RenderContext RenderEngine::renderContext;

    void RenderEngine::Create()
    {
        s_Instance = std::make_unique<RenderEngine>();
        WindowManager::Create();
        RenderAPI::Create();
        Renderer::Create();
        FrameBufferManager::Create();
    }

    void RenderEngine::Update()
    {
    }    
    
    void RenderEngine::BeginRender()
    {
        RenderAPI::GetInstance().BeginFrame();
        //Renderer::GetInstance().BeginFrame();
        renderContext.Reset();
    }    
    
    void RenderEngine::OnResize(int width, int height)
    {
        RenderAPI::GetInstance().OnResize(width, height);
    }    
    
    void RenderEngine::Render()
    {
        // RenderEngine要做的事情，
        // 进行Culling，生成Camcalling 和 LightCulling，
        // 将这些获得的数据放在Context里面， 
        // 然后针对pass去draw， 每个pass自己去配置需要的RT，自己去获取需要的上下文数据。



        //RenderAPI::GetInstance().Render();
        Camera* cam = SceneManager::GetInstance().mainCameraGo->GetComponent<Camera>();
        
        Culling::Run(cam, renderContext);
        
        // 极简...  先不做unity那种根据相机判断。
        // todo：scenecamera的剔除在这个地方做？，那么阴影相机呢？，
        // 需要更加细化？ 暂时不搞，等到下个版本， 目前就是Renderpass。
        // 那么在此处做好剔除，
        Renderer::GetInstance().Render(renderContext);
    }

    void RenderEngine::EndRender()
    {
        renderContext.Reset();
        //Renderer::GetInstance().EndFrame();
        RenderAPI::GetInstance().EndFrame();
    }
}
