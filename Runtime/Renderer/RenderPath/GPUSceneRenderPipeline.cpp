#include "PreCompiledHeader.h"
#include "GPUSceneRenderPipeline.h"
#include "Renderer/RenderEngine.h"

namespace EngineCore
{
    GPUSceneRenderPipeline::GPUSceneRenderPipeline()
    {
        BufferDesc desc;

        desc.debugName = L"CullingParamBuffer";
        desc.memoryType = BufferMemoryType::Upload;
        desc.size = sizeof(GPUCullingParam);
        desc.stride = sizeof(GPUCullingParam);
        desc.usage = BufferUsage::ConstantBuffer;
        for (int i = 0; i < 3; i++) 
        {
            cullingParamBuffer[i] = new GPUBufferAllocator(desc);
        
        }
        cullingParamAlloc = cullingParamBuffer[0]->Allocate(sizeof(GPUCullingParam));


        desc.debugName = L"IndirectDrawArgsBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = sizeof(DrawIndirectArgs) * 3000;
        desc.stride = sizeof(DrawIndirectArgs);
        desc.usage = BufferUsage::StructuredBuffer;
        for (int i = 0; i < 3; i++) 
        {
            indirectDrawArgsBuffer[i] = new GPUBufferAllocator(desc);
        }
        indirectDrawArgsAlloc = indirectDrawArgsBuffer[0]->Allocate(sizeof(DrawIndirectArgs) * 3000);

    }

    void GPUSceneRenderPipeline::Prepare(RenderContext& context)
    {
        PROFILER_ZONE("GPUSceneRenderPipeline::Prepare");
        uint32_t frameID = RenderEngine::GetInstance()->GetCurrentFrame();
        currCullingParamBuffer = GetCurrentCullingParamBuffer(frameID);
        cullingParamAlloc.buffer = currCullingParamBuffer->GetGPUBuffer();
        cullingParamAlloc.offset = 0;
        cullingParamAlloc.gpuAddress = currCullingParamBuffer->GetBaseGPUAddress();

        currIndirectDrawArgsBuffer = GetCurrentIndirectDrawArgsBuffer(frameID);
        indirectDrawArgsAlloc.buffer = currIndirectDrawArgsBuffer->GetGPUBuffer();
        indirectDrawArgsAlloc.offset = 0;
        indirectDrawArgsAlloc.gpuAddress = currIndirectDrawArgsBuffer->GetBaseGPUAddress();
        

        // Get Current BatchInfo:
        vector<DrawIndirectArgs> batchInfo = BatchManager::GetInstance()->GetBatchInfo();
        if (batchInfo.size() != 0) 
        {
            currIndirectDrawArgsBuffer->UploadBuffer(indirectDrawArgsAlloc, batchInfo.data(), batchInfo.size() * sizeof(DrawIndirectArgs));
        }

        auto* visibilityBuffer = RenderEngine::GetInstance()->GetGPUScene().GetCurrentVisibilityBuffer(frameID);
        RenderBackend::GetInstance()->SetResourceState(visibilityBuffer, BufferResourceState::STATE_UNORDERED_ACCESS);
        RenderBackend::GetInstance()->SetResourceState(currIndirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);


        Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
        int gameObjectCount = SceneManager::GetInstance()->GetCurrentScene()->allObjList.size();
        cam->Update();
        context.camera = cam;
        
        GPUCullingParam cullingParam;
        cullingParam.frustum = cam->mFrustum;
        cullingParam.totalItem = gameObjectCount;
        currCullingParamBuffer->UploadBuffer(cullingParamAlloc, &cullingParam, sizeof(GPUCullingParam));

        


        auto& GPUScene = RenderEngine::GetInstance()->GetGPUScene();
        ComputeShader* csShader = GPUScene.GetCullingShaderHandler().Get();
        csShader->SetBuffer("CullingParams", currCullingParamBuffer->GetGPUBuffer());

        auto* allAABBBuffer = GPUScene.GetAllAABBDataBuffer();
        csShader->SetBuffer("g_InputAABBs", allAABBBuffer);
        auto* allObjectBuffer = GPUScene.GetAllObjectDataBuffer();
        csShader->SetBuffer("g_InputPerObjectDatas", allObjectBuffer);
        auto* renderProxyBuffer = GPUScene.GetRenderProxyBuffer();
        csShader->SetBuffer("g_RenderProxies", renderProxyBuffer);
        csShader->SetBuffer("g_VisibleInstanceIndices", visibilityBuffer);
        csShader->SetBuffer("g_IndirectDrawCallArgs", currIndirectDrawArgsBuffer->GetGPUBuffer());

        
        Payload_DispatchComputeShader payload;
        payload.csShader = RenderEngine::GetInstance()->GetGPUScene().GetCullingShaderHandler().Get();
        payload.groupX = gameObjectCount / 64 + 1;
        payload.groupY = 1;
        payload.groupZ = 1;
        RenderBackend::GetInstance()->DispatchComputeShader(payload);
        // 先把shader跑起来， 渲染到RT上， 后续blit啥的接入后面再说

        // 这个地方简单跑个绑定测试？
        RenderBackend::GetInstance()->SetResourceState(visibilityBuffer, BufferResourceState::STATE_SHADER_RESOURCE);
        RenderBackend::GetInstance()->SetResourceState(currIndirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_INDIRECT_ARGUMENT);

        context.IndirectDrawArgsBuffer = currIndirectDrawArgsBuffer->GetGPUBuffer();
        context.CullingParamBuffer = currCullingParamBuffer->GetGPUBuffer();
        for(auto* pass : context.camera->mRenderPassAsset.renderPasses)
        {
            RenderBackend::GetInstance()->FlushPerPassData(context);
            pass->Configure(context);
            pass->Filter(context);
            pass->Prepare(context);
        }
    }


    void GPUSceneRenderPipeline::RecordAndFlush(RenderContext& context)
    {

        //PROFILER_ZONE("MainThread::Renderer::Render")
        for(auto* pass : context.camera->mRenderPassAsset.renderPasses)
        {
            pass->Execute(context);
            RenderBackend::GetInstance()->TryWakeUpRenderThread();
        }
        
        for (auto& pass : context.camera->mRenderPassAsset.renderPasses)
        {
            pass->Clear();
        }
    }

    GPUBufferAllocator* GPUSceneRenderPipeline::GetCurrentCullingParamBuffer(uint32_t frameID)
    {
        int index = frameID % 3;
        return cullingParamBuffer[index];
    } 

    GPUBufferAllocator* GPUSceneRenderPipeline::GetCurrentIndirectDrawArgsBuffer(uint32_t frameID)
    {
        int index = frameID % 3;
        return indirectDrawArgsBuffer[index];
    }
}