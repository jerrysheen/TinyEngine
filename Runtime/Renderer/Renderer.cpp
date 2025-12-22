#include "PreCompiledHeader.h"
#include "Renderer.h"
#include "RenderAPI.h"
#include "Core/PublicEnum.h"
#include "Graphics/FrameBufferObject.h"
#include "Core/Concurrency/CpuEvent.h"

namespace EngineCore
{
    std::unique_ptr<Renderer> Renderer::s_Instance = nullptr;

    Renderer::~Renderer()
    {
        std::cout << "Renderer shutting down, stopping render thread..." << std::endl;
        
        // 1. 设置停止标志
        mRunning.store(false, std::memory_order_release);
        
        // 2. 唤醒可能在等待的渲染线程
        mSleepRenderThreadCV.notify_all();
        // 3. 等待渲染线程结束
        if (mRenderThread.joinable())
        {
            mRenderThread.join();
            std::cout << "Render thread joined successfully." << std::endl;
        }
    }

    void Renderer::Create()
    {
        s_Instance = std::make_unique<Renderer>();
        s_Instance->CreatePerFrameData();
        s_Instance->CreatePerPassForwardData();
    }


    void Renderer::BeginFrame()
    {        
        RenderAPI::GetInstance()->ClearRenderPassInfo();

        DrawCommand temp;
        temp.op = RenderOp::kBeginFrame;
        mRenderBuffer.PushBlocking(temp);   
    }

    void Renderer::Render(RenderContext& context)
    {
        PROFILER_ZONE("MainThread::Renderer::Render");

        FlushPerFrameData();
        for(auto& pass : context.camera->mRenderPassAsset.renderPasses)
        {
            FlushPerPassData(context);
            pass->Configure(context);
            pass->Filter(context);
            pass->Execute(context);
            Submit(pass->GetRenderPassInfo());
        }
        
        for (auto& pass : context.camera->mRenderPassAsset.renderPasses)
        {
            pass->Clear();
        }
       
    }

    void Renderer::EndFrame()
    {
        DrawCommand temp;
        temp.op = RenderOp::kEndFrame;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::FlushPerFrameData()
    {
        mPerFrameData.AmbientColor = Vector3(1.0f, 1.0f, 0.0f);
        RenderAPI::GetInstance()->SetGlobalValue<PerFrameData>((uint32_t)UniformBufferType::PerFrameData, 0, &mPerFrameData);
        SetPerFrameData((UINT)RootSigSlot::PerFrameData);
    }

    void Renderer::FlushPerPassData(const RenderContext &context)
    {
        mPerPassData_Forward.ProjectionMatrix = context.camera->mProjectionMatrix;
        mPerPassData_Forward.ViewMatrix = context.camera->mViewMatrix;
        RenderAPI::GetInstance()->SetGlobalValue<PerPassData_Forward>((uint32_t)UniformBufferType::PerPassData_Foraward, 0, &mPerPassData_Forward);
        SetPerPassData((UINT)RootSigSlot::PerPassData);
    }

    void Renderer::CreatePerFrameData()
    {
        RenderAPI::GetInstance()->CreateGlobalConstantBuffer((uint32_t)UniformBufferType::PerFrameData, sizeof(mPerFrameData));
    }

    void Renderer::CreatePerPassForwardData()
    {
        RenderAPI::GetInstance()->CreateGlobalConstantBuffer((uint32_t)UniformBufferType::PerPassData_Foraward, sizeof(mPerPassData_Forward));
    }

    // 从PSOManager创建一个PSO，可以复用
    void Renderer::SetRenderState(const Material* mat, const RenderPassInfo &passinfo)
    {
        //RenderAPI::GetInstance()->GetOrCreatePSO(mat, passinfo);
        DrawCommand temp;
        temp.op = RenderOp::kSetRenderState;
        PSODesc pso;
        
        pso.colorAttachment = passinfo.colorAttachment.IsValid() ? TextureFormat::R8G8B8A8 : TextureFormat::EMPTY;
        pso.depthAttachment = passinfo.depthAttachment.IsValid() ? TextureFormat::D24S8 : TextureFormat::EMPTY;
        pso.matRenderState = mat->GetMaterialRenderState();
        temp.data.setRenderState.psoDesc = pso;
        
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::SetMaterialData(const Material* mat)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetMaterial;
        temp.data.setMaterial.matId = mat->GetInstanceID();
        temp.data.setMaterial.shader = mat->mShader.Get();
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::ConfigureRenderTarget(const RenderPassInfo &info)
    {
        DrawCommand temp;
        temp.op = RenderOp::kConfigureRT;
        Payload_ConfigureRT& configureRT = temp.data.configureRT;
        // todo :  string name -> id name;
        configureRT.colorAttachment = info.colorAttachment.IsValid() ? info.colorAttachment.Get()->GetInstanceID() : 0;
        configureRT.depthAttachment = info.depthAttachment.IsValid() ? info.depthAttachment.Get()->GetInstanceID() : 0;
        configureRT.isBackBuffer = info.colorAttachment.IsValid() && info.colorAttachment.Get()->mTextureName == "BackBuffer";
        // todo : 对齐两部分指令
        ClearValue value = {Vector3(info.clearColorValue.x, info.clearColorValue.y, info.clearColorValue.z),
            info.clearDepthValue, info.clearFlag};
        configureRT.clearValue = std::move(value);
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::SetMeshData(uint32_t vaoID)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetVBIB;
        temp.data.setVBIB.vaoId = vaoID;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::SetViewPort(const Vector2 &viewportStartXY, const Vector2 &viewportEndXY)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetViewPort;
        Payload_SetViewPort& setViewPort = temp.data.setViewPort;
        setViewPort.x = viewportStartXY.x;
        setViewPort.y = viewportStartXY.y;
        setViewPort.w = viewportEndXY.x - viewportStartXY.x;
        setViewPort.h = viewportEndXY.y - viewportStartXY.y;
        mRenderBuffer.PushBlocking(temp);
    }

    // SissorRect 和ViewPort参数不一样， 一个是widthheight ， 一个是start end
    void Renderer::SetSissorRect(const Vector2& viewportStartXY, const Vector2& viewportEndXY)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetSissorRect;
        Payload_SetSissorRect& setSissorRect = temp.data.setSissorRect;
        setSissorRect.x = viewportStartXY.x;
        setSissorRect.y = viewportStartXY.y;
        setSissorRect.w = viewportEndXY.x;
        setSissorRect.h = viewportEndXY.y;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::ResizeWindow(int width, int height)
    {
        DrawCommand temp;
        temp.op = RenderOp::kWindowResize;
        temp.data.onWindowResize.width = width;
        temp.data.onWindowResize.height = height;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::OnDrawGUI()
    {
        DrawCommand temp;
        temp.op = RenderOp::kIssueEditorGUIDraw;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::SetPerDrawData(const PerDrawHandle &perDrawHandle)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetPerDrawData;
        temp.data.setPerDrawData.perDrawOffset = perDrawHandle.offset;
        temp.data.setPerDrawData.perDrawSize= perDrawHandle.size;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::DrawIndexedInstanced(uint32_t vaoID, int count, const PerDrawHandle &perDrawHandle)
    {
        DrawCommand temp;
        temp.op = RenderOp::kDrawInstanced;
        temp.data.setDrawInstanceCmd.vaoID = vaoID;
        temp.data.setDrawInstanceCmd.count = count;
        temp.data.setDrawInstanceCmd.perDrawOffset = perDrawHandle.offset / sizeof(uint32_t);
        temp.data.setDrawInstanceCmd.perDrawStride = perDrawHandle.size / count;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::SetPerPassData(UINT perPassBufferID)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetPerPassData;
        temp.data.setPerPassData.perPassBufferID = perPassBufferID;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::SetPerFrameData(UINT perFrameBufferID)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetPerFrameData;
        temp.data.setPerFrameData.perFrameBufferID = perFrameBufferID;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::CopyBufferRegion(const Payload_CopyBufferRegion &copyCmd)
    {
        DrawCommand temp;
        temp.op = RenderOp::kCopyBufferRegion;
        temp.data.copyBufferRegion = copyCmd;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::Submit(const RenderPassInfo &info)
    {
        // todo： 后面挪到别的地方， 先做Batch的部分：
        ConfigureRenderTarget(info);
        SetViewPort(info.viewportStartPos, info.viewportEndPos);
        SetSissorRect(info.viewportStartPos, info.viewportEndPos);
        
        SetPerPassData((UINT)info.mRootSigSlot);
        if (info.enableBatch == false) 
        {
            for each(auto& record in info.drawRecordList)
            {
                // 根据mat + pass信息组织pippeline
                SetRenderState(record.mat, info);
                // copy gpu material data desc 
                SetMaterialData(record.mat);
                // bind mesh vertexbuffer and indexbuffer.
                SetMeshData(record.vaoID);

                DrawIndexedInstanced(record.vaoID, record.instanceCount, record.perDrawHandle);
            }
        }
        else 
        {
            for each(auto& record in info.renderBatchList)
            {
                // 根据mat + pass信息组织pippeline
                SetRenderState(record.mat, info);
                // copy gpu material data desc 
                SetMaterialData(record.mat);
                // bind mesh vertexbuffer and indexbuffer.
                SetMeshData(record.vaoID);

                DrawIndexedInstanced(record.vaoID, record.instanceCount, PerDrawHandle{nullptr, (uint32_t)record.alloc.offset, 0});
            }
        }
    }

    // 渲染线程消费指令
    void Renderer::ProcessDrawCommand(const DrawCommand &cmd)
    {
        switch (cmd.op)
        {
        case RenderOp::kConfigureRT :
            RenderAPI::GetInstance()->RenderAPIConfigureRT(cmd.data.configureRT);
        break;
        case RenderOp::kSetMaterial :
            RenderAPI::GetInstance()->RenderAPISetMaterial(cmd.data.setMaterial);
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
        case RenderOp::kWindowResize:
            hasResize = true;
            pendingResize = cmd.data.onWindowResize;
            break;
        case RenderOp::kIssueEditorGUIDraw:
            hasDrawGUI = true;
            break;
        default:
            break;
        }

    }


}