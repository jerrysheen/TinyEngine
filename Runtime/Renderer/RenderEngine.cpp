#include "PreCompiledHeader.h"
#include "RenderEngine.h"
#include "RenderAPI.h"
#include "Settings/ProjectSettings.h"
#include "Managers/WindowManager.h"
#include "GameObject/Camera.h"
#include "RenderBackend.h"
#include "RenderPipeLine/OpaqueRenderPass.h"
#include "Platforms/D3D12/D3D12RenderAPI.h"
#include "Scene/SceneManager.h"
#include "Culling.h"
#include "Scene/Scene.h"
#include "Scene/GPUSCene.h"
#include "Renderer/RenderPath/LagacyRenderPipeline.h"
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
        s_Instance->mGPUScene.Create();
        WindowManager::Create();
        RenderAPI::Create();
        RenderBackend::Create();

        s_Instance->mUploadPagePool = new UploadPagePool(16 * 1024 * 1024);
        s_Instance->mGPUScene.SetUploadPagePool(s_Instance->mUploadPagePool);

        if (RenderSettings::s_RenderPath == RenderSettings::RenderPathType::Legacy)
        {
            s_Instance->mCurrentRenderPipeline = new LagacyRenderPipeline();
        }
        else
        {
            s_Instance->mCurrentRenderPipeline = new GPUSceneRenderPipeline();
        }
    }

    void RenderEngine::PrepareFrame(uint32_t frameID, const SceneDelta& sceneDelta)
    {
        WaitForFrameAvaliable(frameID);
        mCurrentFrameID = frameID;
        mCurrFrameTicket = GetCurrentFrameTicket(frameID);

        PROFILER_ZONE("RenderEngine::Update");
        mCPUScene.Update(frameID);
        mGPUScene.Update(frameID);
        ComsumeDirtySceneRenderNode(sceneDelta);
        //PROFILER_EVENT_BEGIN("MainThread::RenderEngine::Update");
        mCurrentRenderPipeline->Prepare(renderContext);
        UploadCopyOp();
        //PROFILER_EVENT_END("MainThread::RenderEngine::Update");
    }    
    
    void RenderEngine::OnResize(int width, int height)
    {
        RenderBackend::GetInstance()->ResizeWindow(width, height);
    }

    void RenderEngine::BuildFrame()
    {
        mCurrentRenderPipeline->RecordAndFlush(renderContext);
    }

    void RenderEngine::WaitForFrameAvaliable(uint32_t frameID)
    {
        PROFILER_ZONE("MainThread::WaitForFrameContextAvaliable()");
        const FrameTicket* currentTicket = GetCurrentFrameTicket(frameID);
        // 当开始复用 ring 槽位时，必须确保“上一任帧”的提交信息已经由渲染线程发布。
        // 仅等 fence 值不够，因为渲染线程可能还未写入该槽位的最新 fence。
        if (frameID >= MAX_FRAME_INFLIGHT)
        {
            const uint64_t expectedSubmittedFrameID = static_cast<uint64_t>(frameID - MAX_FRAME_INFLIGHT);

            while (!currentTicket->IsSubmissionReadyForFrame(expectedSubmittedFrameID))
            {
                std::this_thread::yield();
            }

            const uint64_t frameFenceValue = currentTicket->GetFenceValue();
            while (RenderAPI::GetInstance()->GetCurrentGPUCompletedFenceValue() < frameFenceValue)
            {
                std::this_thread::yield();
            }
        }

#ifdef EDITOR
        // 等待渲染线程消费完上一帧的 ImGui DrawData
        // 防止 ImGui::NewFrame() 提前清掉渲染线程还在用的 DrawData
        CpuEvent::GUIDataConsumed().Wait();
#endif
    }

    void RenderEngine::EndFrame()
    {
        mCPUScene.EndFrame();
    }

    void RenderEngine::Destory()
    {
        // Renderer 的析构函数会自动停止渲染线程
        RenderBackend::Destroy();
        //RenderAPI::Destroy();
        WindowManager::Destroy();
        RenderEngine::GetInstance()->GetGPUScene().Destroy();

        delete s_Instance->mUploadPagePool;
        delete s_Instance->mCurrentRenderPipeline;
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

    // void RenderEngine::WaitForLastFrameFinished()
    // {
    //     PROFILER_EVENT_BEGIN("MainThread::WaitforSignalFromRenderThread");
    //     CpuEvent::RenderThreadSubmited().Wait();
    //     PROFILER_EVENT_END("MainThread::WaitforSignalFromRenderThread");

    // }

    // void RenderEngine::SignalMainThreadSubmited()
    // {
    //     CpuEvent::MainThreadSubmited().Signal();
    // }

    void RenderEngine::ComsumeDirtySceneRenderNode(const SceneDelta& sceneDelta)
    {
        const std::vector<uint32_t>& mPerFrameDirtyNodeList = sceneDelta.GetPerFrameDirtyNodeList();
        const std::vector<uint32_t>& mNodeChangeFlagList = sceneDelta.GetNodeChangeFlagList();
        const std::vector<NodeDirtyPayload>& mNodeDirtyPayloadList = sceneDelta.GetNodeDirtyPayloadList();
        
        for(uint32_t dirtyRenderID : mPerFrameDirtyNodeList)
        {
            mCPUScene.ApplyDirtyNode(dirtyRenderID, (NodeDirtyFlags)mNodeChangeFlagList[dirtyRenderID], mNodeDirtyPayloadList[dirtyRenderID]);
        }

        CPUSceneView view = mCPUScene.GetSceneView();
        for(uint32_t dirtyRenderID : mPerFrameDirtyNodeList)
        {
            mGPUScene.ApplyDirtyNode(dirtyRenderID, mNodeChangeFlagList[dirtyRenderID], view);
        }
        mGPUScene.UpdatePerFrameDirtyNode(view);
    }

    void RenderEngine::UploadCopyOp()
    {
        mGPUScene.UploadCopyOp();
    }
}
