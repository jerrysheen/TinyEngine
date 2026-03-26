#pragma once
#include "Managers/Manager.h"
#include "RenderPipeLine/RenderPass.h"
#include "Core/PublicStruct.h"
#include "RenderContext.h"
#include <thread>
#include <atomic>
#include <iostream>
#include <condition_variable>
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "RenderCommand.h"
#include "SPSCRingBuffer.h"
#include "RenderAPI.h"
#include <chrono>
#include "RenderUniforms.h"
#include "Renderer/UploadPagePool.h"


#ifdef EDITOR
#include "EditorGUIManager.h"
#endif

#include "Core/Concurrency/CpuEvent.h"


namespace EngineCore
{
    class RenderBackend : public Manager<RenderBackend>
    {
    public:
        RenderBackend(): mRenderThread(&RenderBackend::RenderThreadMain, this), mRunning(true), mPerFrameData{}{};
        ~RenderBackend();
        static void Create();

        void BeginFrame();
        void EndFrame();

        void DrawIndexed(uint32_t vaoID, int count);
        void ResizeWindow(int width, int height);
        void OnDrawGUI();
        void SetPerDrawData(const PerDrawHandle& perDrawHandle);
        void DrawIndexedInstanced(Mesh* mesh, int count, const PerDrawHandle& perDrawHandle);
        void SetPerFrameData(UINT perFrameBufferID);
        void SetPerPassData(UINT perPassBufferID);
        void SetFrame(FrameTicket* frameTicket, uint32_t frameID);
        
        void SetRenderState(const Material* mat, const RenderPassInfo &passinfo);

        void SetMaterialData(Material* mat);

        void ConfigureRenderTarget(const RenderPassInfo& passInfo);

        void SetMeshData(Mesh* meshFilter);

        void SetViewPort(const Vector2& viewportStartXY, const Vector2& viewportEndXY);
        // todo: complete this..
        void SetSissorRect(const Vector2& viewportStartXY, const Vector2& viewportEndXY);

        void ProcessDrawCommand(const DrawCommand& cmd);

        void CopyBufferRegion(const Payload_CopyBufferRegion& copyCmd);

        void DispatchComputeShader(const Payload_DispatchComputeShader& dispatchCmd);
        
        void SetResourceState(IGPUResource* resource, BufferResourceState state);
        
        void SetBindlessMat(Material* mat);
        void SetBindLessMeshIB(uint32_t id);
        
        void UploadBufferStaged(const BufferAllocation& alloc, void* data, uint32_t size);


        void DrawIndirect(Payload_DrawIndirect payload);
        void FlushPerFrameData();
        void FlushPerPassData(const RenderContext& context);
        void CreatePerFrameData();
        void CreatePerPassForwardData();

        void RecycleStagedBuffer(const FrameTicket* ticket);
        void SubmitStagedBuffer(const FrameTicket* ticket);
        
        void RenderThreadMain() 
        {
            while (mRunning.load(std::memory_order_acquire) == true) 
            {
                PROFILER_ZONE("RenderThread::RenderLoop");

                // PROFILER_EVENT_BEGIN("RenderThread::WaitForSignalFromMainThread");
                // CpuEvent::MainThreadSubmited().Wait();
                // PROFILER_EVENT_END("RenderThread::WaitForSignalFromMainThread");


                PROFILER_EVENT_BEGIN("RenderThread::ProcessDrawComand");
                DrawCommand cmd;
                if (!mRenderBuffer.TryPop(cmd)) 
                {
                    mDataAvailableEvent.Wait();
                    continue;
                }

                bool hasBeginFrame = false;
                bool hasEndFrame = false;
                while (mRunning.load(std::memory_order_acquire) == true)
                {
                    if (cmd.op == RenderOp::kBeginFrame) hasBeginFrame = true;
                    if (cmd.op == RenderOp::kEndFrame)
                    {
                        hasEndFrame = true;
                        break;
                    }

                    ProcessDrawCommand(cmd);

                    if (!mRenderBuffer.TryPop(cmd))
                    {
                        mDataAvailableEvent.Wait();
                        if (!mRunning.load(std::memory_order_acquire)) break;
                        if (!mRenderBuffer.TryPop(cmd)) continue;
                    }
                }
                PROFILER_EVENT_END("RenderThread::ProcessDrawComand");

                if (!hasBeginFrame || !hasEndFrame)
                {
                    continue;
                }
                
                // todo Submit UploadPage 打上帧标签
                // later do Gpu Fence...
                RenderAPI::GetInstance()->RenderAPISubmit();

#ifdef EDITOR          
                PROFILER_EVENT_BEGIN("RenderThread::ProcessEditorGUI");
                if (hasDrawGUI)
                {
                    EngineEditor::EditorGUIManager::GetInstance()->EndFrame();
                    hasDrawGUI = false;
                }
                // ImGui DrawData 已消费完毕，通知主线程可以安全调用 ImGui::NewFrame()
                CpuEvent::GUIDataConsumed().Signal();
                PROFILER_EVENT_END("RenderThread::ProcessEditorGUI");
#endif


                RenderAPI::GetInstance()->RenderAPIPresentFrame();


                if (hasResize)
                {
                    RenderAPI::GetInstance()->RenderAPIWindowResize(pendingResize);
                    hasResize = false;
                    pendingResize = { 0,0 };
                }

            }
        };
        

        void TryWakeUpRenderThread();
    private:
        void EnqueueCommand(const DrawCommand& cmd);
        void WaitForQueueSpace();

        SPSCRingBuffer<DrawCommand, 16384> mRenderBuffer;
        std::thread mRenderThread;
        bool hasResize = false;
        bool hasDrawGUI = false;
        Payload_WindowResize pendingResize = { 0, 0 };

        std::mutex mSleepRenderThreadMutex;
        std::condition_variable mSleepRenderThreadCV;
        std::atomic<bool> mRunning;
        PerFrameData mPerFrameData;
        // todo： 后面可能不应该这么设置？ 丧失灵活性了， 至少保证Core里面的内容改了这边同步
        PerPassData_Forward mPerPassData_Forward;
        UploadPagePool* mStagedBuffer = nullptr;
        CpuEvent mDataAvailableEvent;
        FrameTicket* mCurrentFrameTicket;
    };
}
