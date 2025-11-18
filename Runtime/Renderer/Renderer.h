#pragma once
#include "Managers/Manager.h"
#include "RenderPipeLine/RenderPass.h"
#include "Core/PublicStruct.h"
#include "RenderContext.h"
#include <thread>
#include <atomic>
#include <iostream>
#include <condition_variable>
#include "Graphics/ModelData.h"
#include "Graphics/Material.h"
#include "RenderCommand.h"
#include "SPSCRingBuffer.h"
#include "RenderAPI.h"
#include <chrono>
#include "RenderUniforms.h"

#ifdef EDITOR
#include "EditorGUIManager.h"
#endif


namespace EngineCore
{
    class Renderer : public Manager<Renderer>
    {
    public:


        Renderer(): mRenderThread(&Renderer::RenderLoop, this), mRunning(true), mPerFrameData{}{};
        ~Renderer();
        static void Create();

        void BeginFrame();
        void Render(const RenderContext& context);
        void EndFrame();
        // maybe 一些上一帧的clear操作

        //void BeginFrame(int frame);
        //void EndFrame(int frame);
        // 所有逻辑判断都放在Render层， API层只实现简单的逻辑， 找到对应资源，绑定对应资源。


        void DrawIndexed(uint32_t vaoID, int count);
        void ResizeWindow(int width, int height);
        void OnDrawGUI();
        void SetPerDrawData(const PerDrawHandle& perDrawHandle);
        void DrawIndexedInstanced(uint32_t vaoID, int count, const PerDrawHandle& perDrawHandle);
        
        void SetRenderState(const Material* mat, const RenderPassInfo &passinfo);

        void SetMaterialData(const Material* mat);

        void ConfigureRenderTarget(const RenderPassInfo& passInfo);

        void SetMeshData(const ModelData* modelData);

        void SetViewPort(const Vector2& viewportStartXY, const Vector2& viewportEndXY);
        // todo: complete this..
        void SetSissorRect(const Vector2& viewportStartXY, const Vector2& viewportEndXY);

        inline void AddRenderPassInfo(RenderPassInfo* renderpassInfo)
        {
            //mRenderPassInfo.push_back(renderpassInfo);
        }

        void Submit(const RenderPassInfo& info);

        void ProcessDrawCommand(const DrawCommand& cmd);

        void RenderLoop() 
        {
            Payload_WindowResize pendingResize = {0, 0};
            bool hasResize = false;
            bool hasDrawGUI = false;
            while (mRunning.load(std::memory_order_acquire) == true) 
            {
                DrawCommand cmd;
                // 如果pop不出来东西， 说明没渲染指令需要消费，睡
                if(!mRenderBuffer.TryPop(cmd))
                {
                    std::unique_lock<std::mutex> lk(mSleepRenderThreadMutex);
                    // 等待1ms的语法
                    mSleepRenderThreadCV.wait_for(lk, std::chrono::milliseconds(1));
                    // 为什么这个地方要用 continue？
                    continue;
                }

                if(cmd.op == RenderOp::kWindowResize)
                {
                    hasResize = true;
                    pendingResize = cmd.data.onWindowResize;
                }

                if(cmd.op == RenderOp::kIssueEditorGUIDraw)
                {
                    hasDrawGUI = true;
                }

                if(cmd.op == RenderOp::kBeginFrame)
                {
                    currState = RenderOp::kBeginFrame;
                    RenderAPI::GetInstance()->RenderAPIBeginFrame();
                }

                if(cmd.op == RenderOp::kEndFrame)
                {
                    currState = RenderOp::kEndFrame;
                    RenderAPI::GetInstance()->RenderAPISubmit();

#ifdef EDITOR                   
                    if (hasDrawGUI)
                    {
                        EngineEditor::EditorGUIManager::GetInstance()->BeginFrame();
                        EngineEditor::EditorGUIManager::GetInstance()->Render();
                        EngineEditor::EditorGUIManager::GetInstance()->EndFrame();
                        hasDrawGUI = false;
                    }
#endif
                    RenderAPI::GetInstance()->RenderAPIPresentFrame();
                    if(hasResize)
                    {
                        RenderAPI::GetInstance()->RenderAPIWindowResize(pendingResize);
                        hasResize = false;
                    }
                }
                
                if(currState == RenderOp::kBeginFrame)
                    ProcessDrawCommand(cmd);

                cout << "Thread Runing!!!" << endl;
            }
        };


    private:
        SPSCRingBuffer<1024> mRenderBuffer;
        std::thread mRenderThread;
        
        RenderOp currState = RenderOp::kInvalid;
        std::mutex mSleepRenderThreadMutex;
        std::condition_variable mSleepRenderThreadCV;
        std::atomic<bool> mRunning;
        PerFrameData mPerFrameData;

        void FlushPerFrameData();
        void CreatePerFrameData();

    };
}