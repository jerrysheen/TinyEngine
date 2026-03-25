#include "PreCompiledHeader.h"
#include "RenderBackend.h"
#include "RenderAPI.h"
#include "Core/PublicEnum.h"
#include "Core/Concurrency/CpuEvent.h"
#include "Graphics/GPUBufferAllocator.h"
#include "Renderer/RenderEngine.h"
namespace EngineCore
{
    std::unique_ptr<RenderBackend> RenderBackend::s_Instance = nullptr;

    RenderBackend::~RenderBackend()
    {
        std::cout << "Renderer shutting down, stopping render thread..." << std::endl;
        
        // 1. 设置停止标志
        mRunning.store(false, std::memory_order_release);
        
        // 2. 唤醒可能在等待的渲染线程
        mDataAvailableEvent.Signal();
        mSleepRenderThreadCV.notify_all();
        // 3. 等待渲染线程结束
        if (mRenderThread.joinable())
        {
            mRenderThread.join();
            std::cout << "Render thread joined successfully." << std::endl;
        }
    }

    void RenderBackend::Create()
    {
        s_Instance = std::make_unique<RenderBackend>();
        s_Instance->CreatePerFrameData();
        s_Instance->CreatePerPassForwardData();
        s_Instance->mStagedBuffer = new UploadPagePool(16 * 1024 * 1024);
        s_Instance->mStagedBuffer->AcquireFreePage();
    }


    void RenderBackend::BeginFrame()
    {        
        DrawCommand temp;
        temp.op = RenderOp::kBeginFrame;
        EnqueueCommand(temp);
    }


    void RenderBackend::EndFrame()
    {
        DrawCommand temp;
        temp.op = RenderOp::kEndFrame;
        EnqueueCommand(temp);
        TryWakeUpRenderThread();
    }

    void RenderBackend::EnqueueCommand(const DrawCommand& cmd)
    {
        while (!mRenderBuffer.TryPush(cmd)) 
        {
            mDataAvailableEvent.Signal();
            WaitForQueueSpace();
        }
    }

    void RenderBackend::TryWakeUpRenderThread()
    {
        mDataAvailableEvent.Signal();
    }

    void RenderBackend::WaitForQueueSpace()
    {
        //// 你后续可替换成 spin -> yield -> wait 的三级退避
    }

    void RenderBackend::FlushPerFrameData()
    {
        mPerFrameData.AmbientColor = Vector3(1.0f, 1.0f, 0.0f);
        RenderAPI::GetInstance()->SetGlobalValue<PerFrameData>((uint32_t)UniformBufferType::PerFrameData, 0, &mPerFrameData);
        SetPerFrameData((UINT)RootSigSlot::PerFrameData);
    }

    void RenderBackend::FlushPerPassData(const RenderContext &context)
    {
        mPerPassData_Forward.ProjectionMatrix = context.camera->mProjectionMatrix;
        mPerPassData_Forward.ViewMatrix = context.camera->mViewMatrix;
        RenderAPI::GetInstance()->SetGlobalValue<PerPassData_Forward>((uint32_t)UniformBufferType::PerPassData_Foraward, 0, &mPerPassData_Forward);
        SetPerPassData((UINT)RootSigSlot::PerPassData);
    }

    void RenderBackend::CreatePerFrameData()
    {
        RenderAPI::GetInstance()->CreateGlobalConstantBuffer((uint32_t)UniformBufferType::PerFrameData, sizeof(mPerFrameData));
    }

    void RenderBackend::CreatePerPassForwardData()
    {
        RenderAPI::GetInstance()->CreateGlobalConstantBuffer((uint32_t)UniformBufferType::PerPassData_Foraward, sizeof(mPerPassData_Forward));
    }

    // 根据传入的FrameTicket重置，
    void RenderBackend::ResetByLastFrameTicket(Payload_SetFrame setFrame)
    {
        mStagedBuffer->Recycle(*setFrame.frameTicket);
    }

    // 从PSOManager创建一个PSO，可以复用
    void RenderBackend::SetRenderState(const Material* mat, const RenderPassInfo &passinfo)
    {
        //RenderAPI::GetInstance()->GetOrCreatePSO(mat, passinfo);
        DrawCommand temp;
        temp.op = RenderOp::kSetRenderState;
        PSODesc pso;
        
        pso.colorAttachment = passinfo.colorAttachment ? TextureFormat::R8G8B8A8 : TextureFormat::EMPTY;
        pso.depthAttachment = passinfo.depthAttachment ? TextureFormat::D24S8 : TextureFormat::EMPTY;
        pso.matRenderState = mat->GetMaterialRenderState();
        temp.data.setRenderState.psoDesc = pso;
        
        EnqueueCommand(temp);
    }

    void RenderBackend::SetMaterialData(Material* mat)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetMaterial;
        temp.data.setMaterial.mat = mat;
        temp.data.setMaterial.shader = mat->mShader.Get();
        EnqueueCommand(temp);

    }

    void RenderBackend::ConfigureRenderTarget(const RenderPassInfo &info)
    {
        DrawCommand temp;
        temp.op = RenderOp::kConfigureRT;
        Payload_ConfigureRT& configureRT = temp.data.configureRT;
        // todo :  string name -> id name;
        configureRT.colorAttachment = info.colorAttachment ? info.colorAttachment->textureBuffer : nullptr;
        configureRT.depthAttachment = info.depthAttachment ? info.depthAttachment->textureBuffer : nullptr;
        // todo : 对齐两部分指令
        ClearValue value = {Vector3(info.clearColorValue.x, info.clearColorValue.y, info.clearColorValue.z),
            info.clearDepthValue, info.clearFlag};
        configureRT.clearValue = std::move(value);
        EnqueueCommand(temp);
    }

    void RenderBackend::SetMeshData(Mesh* mesh)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetVBIB;
        temp.data.setVBIB.mesh = mesh;
        EnqueueCommand(temp);
    }

    void RenderBackend::SetViewPort(const Vector2 &viewportStartXY, const Vector2 &viewportEndXY)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetViewPort;
        Payload_SetViewPort& setViewPort = temp.data.setViewPort;
        setViewPort.x = viewportStartXY.x;
        setViewPort.y = viewportStartXY.y;
        setViewPort.w = viewportEndXY.x - viewportStartXY.x;
        setViewPort.h = viewportEndXY.y - viewportStartXY.y;
        EnqueueCommand(temp);
    }

    // SissorRect 和ViewPort参数不一样， 一个是widthheight ， 一个是start end
    void RenderBackend::SetSissorRect(const Vector2& viewportStartXY, const Vector2& viewportEndXY)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetSissorRect;
        Payload_SetSissorRect& setSissorRect = temp.data.setSissorRect;
        setSissorRect.x = viewportStartXY.x;
        setSissorRect.y = viewportStartXY.y;
        setSissorRect.w = viewportEndXY.x;
        setSissorRect.h = viewportEndXY.y;
        EnqueueCommand(temp);
    }

    void RenderBackend::ResizeWindow(int width, int height)
    {
        DrawCommand temp;
        temp.op = RenderOp::kWindowResize;
        temp.data.onWindowResize.width = width;
        temp.data.onWindowResize.height = height;
        EnqueueCommand(temp);
    }

    void RenderBackend::OnDrawGUI()
    {
        DrawCommand temp;
        temp.op = RenderOp::kIssueEditorGUIDraw;
        EnqueueCommand(temp);
    }

    void RenderBackend::SetPerDrawData(const PerDrawHandle &perDrawHandle)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetPerDrawData;
        temp.data.setPerDrawData.perDrawOffset = perDrawHandle.offset;
        temp.data.setPerDrawData.perDrawSize= perDrawHandle.size;
        EnqueueCommand(temp);
    }

    void RenderBackend::DrawIndexedInstanced(Mesh* mesh, int count, const PerDrawHandle &perDrawHandle)
    {
        DrawCommand temp;
        temp.op = RenderOp::kDrawInstanced;
        temp.data.setDrawInstanceCmd.mesh = mesh;
        temp.data.setDrawInstanceCmd.count = count;
        temp.data.setDrawInstanceCmd.perDrawOffset = perDrawHandle.offset / sizeof(uint32_t);
        temp.data.setDrawInstanceCmd.perDrawStride = perDrawHandle.size / count;
        EnqueueCommand(temp);
    }

    void RenderBackend::SetPerPassData(UINT perPassBufferID)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetPerPassData;
        temp.data.setPerPassData.perPassBufferID = perPassBufferID;
        EnqueueCommand(temp);
    }

    void RenderBackend::SetPerFrameData(UINT perFrameBufferID)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetPerFrameData;
        temp.data.setPerFrameData.perFrameBufferID = perFrameBufferID;
        EnqueueCommand(temp);
    }

    void RenderBackend::SetFrame(FrameTicket* frameTicket, uint32_t frameID)
    {
        ASSERT(frameTicket != nullptr);
        DrawCommand temp;
        temp.op = RenderOp::kSetFrame;
        temp.data.setFrame.frameTicket = frameTicket;
        temp.data.setFrame.frameID = frameID;
        EnqueueCommand(temp);
    }

    void RenderBackend::CopyBufferRegion(const Payload_CopyBufferRegion &copyCmd)
    {
        DrawCommand temp;
        temp.op = RenderOp::kCopyBufferRegion;
        temp.data.copyBufferRegion = copyCmd;
        EnqueueCommand(temp);
    }

    void RenderBackend::DispatchComputeShader(const Payload_DispatchComputeShader &dispatchCmd)
    {
        DrawCommand cmd;
        cmd.data.dispatchComputeShader = dispatchCmd;
        cmd.op = RenderOp::kDispatchComputeShader;
        EnqueueCommand(cmd);
    }

    void RenderBackend::SetResourceState(IGPUResource* resource, BufferResourceState state)
    {
        Payload_SetBufferResourceState payload;
        payload.resource = resource;
        payload.state = state;
        DrawCommand cmd;
        cmd.data.setBufferResourceState = payload;
        cmd.op = RenderOp::kSetBufferResourceState;
        EnqueueCommand(cmd);
    }

    void RenderBackend::SetBindlessMat(Material *mat)
    {
        Payload_SetBindlessMat payload;
        payload.mat = mat;
        DrawCommand cmd;
        cmd.data.setBindlessMat = payload;
        cmd.op = RenderOp::kSetBindlessMat;
        EnqueueCommand(cmd);
    }

    void RenderBackend::SetBindLessMeshIB(uint32_t id)
    {
        Payload_SetBindLessMeshIB payload;
        payload.id = id;
        DrawCommand cmd;
        cmd.data.SetBindLessMeshIB = payload;
        cmd.op = RenderOp::kSetBindLessMeshIB;
        EnqueueCommand(cmd);
    }

    void RenderBackend::UploadBufferStaged(const BufferAllocation& alloc, void* data, uint32_t size)
    {
        Payload_CopyBufferStaged payload;

        BufferAllocation destAllocation = alloc;
        BufferAllocation srcAllocation = mStagedBuffer->Allocate(size, data);

        CopyOp op = {};
        op.srcUploadBuffer = srcAllocation.buffer;
        op.destDefaultBuffer = destAllocation.buffer;
        op.srcOffset = srcAllocation.offset;
        op.dstOffset = destAllocation.offset;
        op.size = size;
        payload.copyOp = op;

        DrawCommand temp;
        temp.op = RenderOp::kCopyBufferStaged;
        temp.data.copyBufferStaged = payload;
        EnqueueCommand(temp);
    }

    void RenderBackend::DrawIndirect(Payload_DrawIndirect payload)
    {
        DrawCommand cmd;
        cmd.data.setDrawIndirect = payload;
        cmd.op = RenderOp::kDrawIndirect;
        EnqueueCommand(cmd);
    }


    // 渲染线程消费指令
    void RenderBackend::ProcessDrawCommand(const DrawCommand &cmd)
    {
        switch (cmd.op)
        {
        case RenderOp::kBeginFrame :
            RenderAPI::GetInstance()->RenderAPIBeginFrame();
        break;
        case RenderOp::kConfigureRT :
            RenderAPI::GetInstance()->RenderAPIConfigureRT(cmd.data.configureRT);
        break;
        case RenderOp::kSetMaterial :
            RenderAPI::GetInstance()->RenderAPISetMaterial(cmd.data.setMaterial);
        break;
        case RenderOp::kSetBindlessMat :
            RenderAPI::GetInstance()->RenderAPISetBindlessMat(cmd.data.setBindlessMat);
        break;
        case RenderOp::kSetBindLessMeshIB :
            RenderAPI::GetInstance()->RenderAPISetBindLessMeshIB();
        break;
        case RenderOp::kCopyBufferRegion :
            RenderAPI::GetInstance()->RenderAPICopyRegion(cmd.data.copyBufferRegion);
        break;
        case RenderOp::kSetRenderState :
            PROFILER_COUNTER_ADD("RenderBatches", 1);
            RenderAPI::GetInstance()->RenderAPISetRenderState(cmd.data.setRenderState);
        break;     
        case RenderOp::kSetSissorRect :
            RenderAPI::GetInstance()->RenderAPISetSissorRect(cmd.data.setSissorRect);
        break;    
        case RenderOp::kSetVBIB :
            RenderAPI::GetInstance()->RenderAPISetVBIB(cmd.data.setVBIB);
        break;     
        case RenderOp::kSetViewPort :
            RenderAPI::GetInstance()->RenderAPISetViewPort(cmd.data.setViewPort);
        break;     
        case RenderOp::kSetPerDrawData :
            RenderAPI::GetInstance()->RenderAPISetPerDrawData(cmd.data.setPerDrawData);
        break;     
        case RenderOp::kDrawInstanced :
            PROFILER_COUNTER_ADD("RenderDrawCalls", 1);
            RenderAPI::GetInstance()->RenderAPIDrawInstanceCmd(cmd.data.setDrawInstanceCmd);
        break;   
        case RenderOp::kSetPerPassData :
            RenderAPI::GetInstance()->RenderAPISetPerPassData(cmd.data.setPerPassData);
        break;     
        case RenderOp::kSetPerFrameData :
            RenderAPI::GetInstance()->RenderAPISetPerFrameData(cmd.data.setPerFrameData);
            break;  
        case RenderOp::kSetFrame :
            // todo: set current Frame
            ResetByLastFrameTicket(cmd.data.setFrame);
            mCurrentFrameTicket = cmd.data.setFrame.frameTicket;
            RenderAPI::GetInstance()->RenderAPISetFrame(cmd.data.setFrame);
            break;
        case RenderOp::kWindowResize:
            hasResize = true;
            pendingResize = cmd.data.onWindowResize;
            break;
        case RenderOp::kIssueEditorGUIDraw:
            hasDrawGUI = true;
            break;
        case RenderOp::kDispatchComputeShader:
            RenderAPI::GetInstance()->RenderAPIDispatchComputeShader(cmd.data.dispatchComputeShader);
            break;
        case RenderOp::kSetBufferResourceState:
            RenderAPI::GetInstance()->RenderAPISetBufferResourceState(cmd.data.setBufferResourceState);
            break;
        case RenderOp::kDrawIndirect:
            RenderAPI::GetInstance()->RenderAPIExecuteIndirect(cmd.data.setDrawIndirect);
            break;
        case RenderOp::kCopyBufferStaged:
            RenderAPI::GetInstance()->RenderAPICopyBufferStaged(cmd.data.copyBufferStaged);
            break;
        default:
            break;
        }

    }


}
