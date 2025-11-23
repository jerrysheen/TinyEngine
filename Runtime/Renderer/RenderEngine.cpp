#include "PreCompiledHeader.h"
#include "RenderEngine.h"
#include "RenderAPI.h"
#include "Managers/WindowManager.h"
#include "GameObject/Camera.h"
#include "Renderer.h"
#include "RenderPipeLine/OpaqueRenderPass.h"
#include "Platforms/D3D12/D3D12RenderAPI.h"
#include "Scene/SceneManager.h"
#include "Culling.h"
#include "FrameBufferManager.h"
#include "Scene/Scene.h"


// RenderEngine 类的设计逻辑：
// 负责创建渲染相关的，比如窗口，渲染API， Renderer，
// 负责拉起每一帧的渲染， Renderer的主线程数据组织， Culling的运行
// 【优化】： 其实这个地方可以变成一个Tick操作， 里面包含begin， Render， end，更加简单。
// RenderEngine不负责直接和RenderAPI沟通，那个在Renderer里面解决
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
    
    void RenderEngine::OnResize(int width, int height)
    {
        Renderer::GetInstance()->ResizeWindow(width, height);
    }

    void RenderEngine::Tick()
    {

        WaitForLastFrameFinished();

        Renderer::GetInstance()->BeginFrame();
        renderContext.Reset();

        // todo： 这个地方culling逻辑是不是应该放到Update
        Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
        Culling::Run(cam, renderContext);

        Renderer::GetInstance()->Render(renderContext);
       
        #ifdef EDITOR
        Renderer::GetInstance()->OnDrawGUI();
        #endif
        
        Renderer::GetInstance()->EndFrame();

        SignalMainThreadSubmited();
    }

    void RenderEngine::Destory()
    {
        // Renderer 的析构函数会自动停止渲染线程
        Renderer::Destroy();
        FrameBufferManager::Destroy();
        //RenderAPI::Destroy();
        WindowManager::Destroy();
        
        // 最后销毁 RenderEngine 自己
        if (s_Instance)
        {
            // 先调用析构（智能指针自动管理）
            s_Instance.reset();
            // 此时：
            // 1. 析构函数已完成
            // 2. 内存已释放
            // 3. s_Instance 已经是 nullptr
            // 4. 后续 GetInstance() 不会返回野指针
        }
    }

    void RenderEngine::WaitForLastFrameFinished()
    {
        PROFILER_EVENT_BEGIN("MainThread::WaitforSignalFromRenderThread");
        CpuEvent::RenderThreadSubmited().Wait();
        PROFILER_EVENT_END("MainThread::WaitforSignalFromRenderThread");

    }

    void RenderEngine::SignalMainThreadSubmited()
    {
        CpuEvent::MainThreadSubmited().Signal();
    }

}
