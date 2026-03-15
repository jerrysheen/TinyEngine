#pragma once
#include "Managers/Manager.h"
#include "RenderContext.h"
#include "Renderer/RenderPath/LagacyRenderPipeline.h"
#include "Renderer/RenderPath/GPUSceneRenderPipeline.h"
#include "Scene/GPUScene.h"
#include "Scene/CPUScene.h"
#include "UploadPagePool.h"

namespace EngineCore
{
    // 平台无关类， 负责拉起循环而已，内部各种渲染类组成。
    // 首先明确下这些函数都在负责什么功能。
    // begin render: RenderAPI->BeginFrame() 等待上一帧结束，OnResize处理
    // render: 执行场景渲染，不同管线进行渲染等。
    // end render : RenderAPI->EndFrame() 调用Present，buckbufferIndex Update
    class RenderEngine
    {
    public:
        static RenderEngine* GetInstance(){return s_Instance.get();};
        static bool IsInitialized(){return s_Instance != nullptr;};
        void PrepareFrame(uint32_t frameID, const SceneDelta& delta);
        static void Create();
        void BuildFrame();
        void WaitForFrameAvaliable(uint32_t frameID);
        void EndFrame();
        
        static void OnResize(int width, int height);
        static void OnDrawGUI();
        static void Destory();
        RenderEngine(){};
        ~RenderEngine(){};
        //static void WaitForLastFrameFinished();
        //static void SignalMainThreadSubmited();
        inline CPUScene& GetCPUScene(){return mCPUScene;}
        inline GPUScene& GetGPUScene(){return mGPUScene;}
        inline void SetCurrentFrame(uint32_t currentFrame)
        {
            mCPUScene.SetCurrentFrame(currentFrame);
            mGPUScene.SetCurrentFrame(currentFrame);
        }

        inline FrameTicket* GetCurrentFrameTicket(uint32_t frameID)
        {
            return &mFrameTicket[frameID % 3];
        }

        inline uint32_t GetCurrentFrame(){ return mCurrentFrameID;}
    private:
        IRenderPipeline* mCurrentRenderPipeline;
        
        static std::unique_ptr<RenderEngine> s_Instance;
        RenderContext renderContext;
        UploadPagePool* mUploadPagePool;
        GPUScene mGPUScene;
        CPUScene mCPUScene;
        static constexpr int  MAX_FRAME_INFLIGHT = 3;
        
        FrameTicket* mCurrFrameTicket;
        FrameTicket mFrameTicket[MAX_FRAME_INFLIGHT];
        uint32_t mCurrentFrameID = 0;

        void ComsumeDirtySceneRenderNode(const SceneDelta& delta);
        void UploadCopyOp();
    };
    
}
