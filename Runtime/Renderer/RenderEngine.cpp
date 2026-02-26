#include "PreCompiledHeader.h"
#include "RenderEngine.h"
#include "RenderAPI.h"
#include "Settings/ProjectSettings.h"
#include "Managers/WindowManager.h"
#include "GameObject/Camera.h"
#include "Renderer.h"
#include "RenderPipeLine/OpaqueRenderPass.h"
#include "Platforms/D3D12/D3D12RenderAPI.h"
#include "Scene/SceneManager.h"
#include "Culling.h"
#include "Scene/Scene.h"
#include "Scene/GPUSCene.h"
#include "Renderer/RenderPath/LagacyRenderPath.h"
// RenderEngine 类的设计逻辑：
// 负责创建渲染相关的，比如窗口，渲染API， Renderer，
// 负责拉起每一帧的渲染， Renderer的主线程数据组织， Culling的运行
// 【优化】： 其实这个地方可以变成一个Tick操作， 里面包含begin， Render， end，更加简单。
// RenderEngine不负责直接和RenderAPI沟通，那个在Renderer里面解决
namespace EngineCore
{
    std::unique_ptr<RenderEngine> RenderEngine::s_Instance = nullptr;
    void RenderEngine::Create()
    {
        s_Instance = std::make_unique<RenderEngine>();
        s_Instance->GetGPUScene().Create();
        WindowManager::Create();
        RenderAPI::Create();
        Renderer::Create();


        if (RenderSettings::s_RenderPath == RenderSettings::RenderPathType::Legacy)
        {
            s_Instance->mCurrentRenderPath = new LagacyRenderPath();
        }
        else
        {
            s_Instance->mCurrentRenderPath = new GPUSceneRenderPath();
        }
    }

    void RenderEngine::Update(uint32_t frameID)
    {

        mCPUScene.Update(frameID);
        mGPUScene.Update(frameID);
        ComsumeDirtySceneRenderNode();
        WaitForLastFrameFinished();
        PROFILER_EVENT_BEGIN("MainThread::WaitForGpuFinished");
        RenderAPI::GetInstance()->WaitForGpuFinished();
        PROFILER_EVENT_END("MainThread::WaitForGpuFinished");
        mCurrentRenderPath->Prepare(renderContext);
        UploadCopyOp();
    }    
    
    void RenderEngine::OnResize(int width, int height)
    {
        Renderer::GetInstance()->ResizeWindow(width, height);
    }

    void RenderEngine::Tick()
    {



        s_Instance->mCurrentRenderPath->Execute(renderContext);
        
        SignalMainThreadSubmited();

    }

    void RenderEngine::EndFrame()
    {
        mCPUScene.EndFrame();
    }

    void RenderEngine::Destory()
    {
        // Renderer 的析构函数会自动停止渲染线程
        Renderer::Destroy();
        //RenderAPI::Destroy();
        WindowManager::Destroy();
        RenderEngine::GetInstance()->GetGPUScene().Destroy();

        delete s_Instance->mCurrentRenderPath;
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

    void RenderEngine::ComsumeDirtySceneRenderNode()
    {
        Scene* scene = SceneManager::GetInstance()->GetCurrentScene();
        std::vector<uint32_t>& mPerFrameDirtyNodeList = scene->GetPerFrameDirtyNodeList();
        std::vector<uint32_t>& mNodeChangeFlagList = scene->GetNodeChangeFlagList();
        std::vector<NodeDirtyPayload>& mNodeDirtyPayloadList = scene->GetNodeDirtyPayloadList();
        
        for(uint32_t dirtyRenderID : mPerFrameDirtyNodeList)
        {
            mCPUScene.ApplyDirtyNode(dirtyRenderID, (NodeDirtyFlags)mNodeChangeFlagList[dirtyRenderID], mNodeDirtyPayloadList[dirtyRenderID]);
        }

        CPUSceneView view = mCPUScene.GetSceneView();
        for(uint32_t dirtyRenderID : mPerFrameDirtyNodeList)
        {
            mGPUScene.ApplyDirtyNode(dirtyRenderID, mNodeChangeFlagList[dirtyRenderID], view);
        }
        mGPUScene.UpdateDirtyNode(view);

    }

    void RenderEngine::UploadCopyOp()
    {
        mGPUScene.UploadCopyOp();
    }
}
