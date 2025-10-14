#include "PreCompiledHeader.h"
#include "Renderer.h"
#include "RenderAPI.h"
#include "Core/PublicEnum.h"


namespace EngineCore
{
    std::unique_ptr<Renderer> Renderer::s_Instance = nullptr;

    void Renderer::Create()
    {
        s_Instance = std::make_unique<Renderer>();
    }


    void Renderer::BeginFrame()
    {
        // Clear RenderInfo in RenderAPI
        // todo: 这个地方是否需要优化
        RenderAPI::GetInstance().ClearRenderPassInfo();

        DrawCommand temp;
        temp.op = RenderOp::kBeginFrame;
        mRenderBuffer.PushBlocking(temp);   
    }

    void Renderer::Render(const RenderContext& context)
    {
        BeginFrame();
        for(auto& pass : context.camera->mRenderPassAsset.renderPasses)
        {
            pass->Configure(context);
            pass->Filter(context);
            pass->Execute(context);
            //pass->Submit();
            
            // 处理每个pass的渲染信息：
            // 感觉不应该这么写， 而是从pass出发，pass去调用上传之类的。
            Submit(pass->GetRenderPassInfo());
        }
        //RenderAPI::GetInstance().Submit(mRenderPassInfo);
        EndFrame();
        // 重置每个pass产生的渲染信息
        for (auto& pass : context.camera->mRenderPassAsset.renderPasses)
        {
            pass->Clear();
        }
    }

    void Renderer::EndFrame()
    {
        // submit data to renderthread.
        //mRenderPassInfo.clear();
        DrawCommand temp;
        temp.op = RenderOp::kEndFrame;
        mRenderBuffer.PushBlocking(temp);
    }
    

    // 从PSOManager创建一个PSO，可以复用
    void Renderer::SetRenderState(const Material* mat, const RenderPassInfo &passinfo)
    {
        //RenderAPI::GetInstance().GetOrCreatePSO(mat, passinfo);
        DrawCommand temp;
        temp.op = RenderOp::kSetRenderState;
        temp.data.setRenderState.psoId = mat->shader->GetInstanceID();
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::SetMaterialData(const Material* mat)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetMaterial;
        temp.data.setMaterial.matId = mat->GetInstanceID();
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::ConfigureRenderTarget(const RenderPassInfo &info)
    {
        DrawCommand temp;
        temp.op = RenderOp::kConfigureRT;
        Payload_ConfigureRT& configureRT = temp.data.configureRT;
        // todo :  string name -> id name;
        configureRT.colorAttachment = info.colorAttachment == nullptr ? 0 : info.colorAttachment->GetInstanceID();
        configureRT.depthAttachment = info.depthAttachment == nullptr ? 0 : info.depthAttachment->GetInstanceID();
        configureRT.isBackBuffer = info.colorAttachment && info.colorAttachment->name == "BackBuffer";
        // todo : 对齐两部分指令
        ClearValue value = {Vector3(0,0,0), 0.0, ClearFlag::All};
        configureRT.clearValue = std::move(value);
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::SetMeshData(const ModelData *modelData)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetVBIB;
        temp.data.setVBIB.vaoId = modelData->VAO;
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

    void Renderer::SetSissorRect(const Vector2& viewportStartXY, const Vector2& viewportEndXY)
    {
        DrawCommand temp;
        temp.op = RenderOp::kSetSissorRect;
        Payload_SetSissorRect& setSissorRect = temp.data.setSissorRect;
        setSissorRect.x = viewportStartXY.x;
        setSissorRect.y = viewportStartXY.y;
        setSissorRect.w = viewportEndXY.x - viewportStartXY.x;
        setSissorRect.h = viewportEndXY.y - viewportStartXY.y;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::DrawIndexed(uint32_t vaoID, int count)
    {
        DrawCommand temp;
        temp.op = RenderOp::kDrawIndexed;
        temp.data.setDrawCmd.count = count;
        temp.data.setDrawCmd.vaoID = vaoID;
        mRenderBuffer.PushBlocking(temp);
    }

    void Renderer::Submit(const RenderPassInfo &info)
    {
        ConfigureRenderTarget(info);
        SetViewPort(info.viewportStartPos, info.viewportEndPos);
        SetSissorRect(info.viewportStartPos, info.viewportEndPos);
        
        for each(auto& record in info.drawRecordList)
        {
            // GetorCreate PSO...
            // 根据mat + pass信息组织pippeline
            // todo: we need batch here...

            SetRenderState(record.mat, info);
            // todo： 这个粒度是否还是太大。。材质中应该涉及到某些block不变才对。

            // copy gpu material data desc 
            SetMaterialData(record.mat);

            // bind mesh vertexbuffer and indexbuffer.
            SetMeshData(record.model);

            DrawIndexed(record.model->VAO, 1);
            // todo :  提交的信号是什么
            // 应该像unity那样
        }
    }

    // 渲染线程消费指令
    void Renderer::ProcessDrawCommand(const DrawCommand &cmd)
    {
        switch (cmd.op)
        {
        case RenderOp::kConfigureRT :
            RenderAPI::GetInstance().RenderAPIConfigureRT(cmd.data.configureRT);
        break;
        case RenderOp::kDrawIndexed :
            RenderAPI::GetInstance().RenderAPIDrawIndexed(cmd.data.setDrawCmd);
        break;        
        case RenderOp::kSetMaterial :
            RenderAPI::GetInstance().RenderAPISetMaterial(cmd.data.setMaterial);
        break;     
        case RenderOp::kSetRenderState :
            RenderAPI::GetInstance().RenderAPISetRenderState(cmd.data.setRenderState);
        break;     
        case RenderOp::kSetSissorRect :
            RenderAPI::GetInstance().RenderAPISetSissorRect(cmd.data.setSissorRect);
        break;    
        case RenderOp::kSetVBIB :
            RenderAPI::GetInstance().RenderAPISetVBIB(cmd.data.setVBIB);
        break;     
        case RenderOp::kSetViewPort :
            RenderAPI::GetInstance().RenderAPISetViewPort(cmd.data.setViewPort);
        break;     
        default:
            break;
        }

    }

}