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
        cullingParamBuffers = new PerFrameBufferRing(desc);

        desc.debugName = L"IndirectDrawSourceBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = sizeof(IndirectDrawSource) * 3000;
        desc.stride = sizeof(IndirectDrawSource);
        desc.usage = BufferUsage::StructuredBuffer;
        indirectDrawSourceBuffers = new PerFrameBufferRing(desc);

        desc.debugName = L"IndirectDrawArgsDestBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = sizeof(IndirectDrawDest) * 3000;
        desc.stride = sizeof(IndirectDrawDest);
        desc.usage = BufferUsage::StructuredBuffer;
        indirectDrawDestBuffers = new PerFrameBufferRing(desc);


        desc.debugName = L"IndirectDrawCountBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = sizeof(uint32_t) * 3000;
        desc.stride = sizeof(uint32_t);
        desc.usage = BufferUsage::ByteAddressBuffer;
        indirectDrawCountBuffers = new PerFrameBufferRing(desc);

        desc.debugName = L"BatchCountBuffer";
        desc.memoryType = BufferMemoryType::Upload;
        desc.size = sizeof(uint32_t);
        desc.stride = sizeof(uint32_t);
        desc.usage = BufferUsage::ConstantBuffer;
        batchCounterBuffers = new PerFrameBufferRing(desc);

        //创建Compute Shader
        string path = PathSettings::ResolveAssetPath("Shader/IndirectDrawCallCombineComputeShader.hlsl");
        IndirectDrawCombineShaderHandler = ResourceManager::GetInstance()->CreateResource<ComputeShader>(path);

        path = PathSettings::ResolveAssetPath("Shader/HIzCSShader.hlsl");
        HizCSShaderHandler = ResourceManager::GetInstance()->CreateResource<ComputeShader>(path);

        path = PathSettings::ResolveAssetPath("Shader/BlitCSShader.hlsl");
        BlitCSShaderHandler = ResourceManager::GetInstance()->CreateResource<ComputeShader>(path);
        
        TextureDesc hiZDesc;
        hiZDesc.name = "hiZTexture";
        hiZDesc.dimension = TextureDimension::TEXTURE2D;
        hiZDesc.width = 1920;
        hiZDesc.height = 1080;
        hiZDesc.format = TextureFormat::R16Float;
        hiZDesc.texUsage = (TextureUsage::UnorderedAccess | TextureUsage::ShaderResource);
        hiZDesc.mipCount = static_cast<uint32_t>(std::floor(std::log2(std::max(hiZDesc.width, hiZDesc.height)))) + 1;
        for(int i = 0; i < 3; i++)
        {
            std::wstring name = L"hiZTexture" + std::to_wstring(i + 1);
            hiZDesc.name = std::string(name.begin(), name.end());;
            RWHiZTexture[i] = new RenderTexture(hiZDesc);
        }
    }

    void GPUSceneRenderPipeline::Prepare(RenderContext& context)
    {
        PROFILER_ZONE("GPUSceneRenderPipeline::Prepare");
        uint32_t frameID = RenderEngine::GetInstance()->GetCurrentFrame();
        GPUBufferAllocator* currCullingParamBuffer = GetCurrentCullingParamBuffer(frameID);
        BufferAllocation cullingParamAlloc;
        cullingParamAlloc.buffer = currCullingParamBuffer->GetGPUBuffer();
        cullingParamAlloc.offset = 0;
        cullingParamAlloc.gpuAddress = currCullingParamBuffer->GetBaseGPUAddress();
        cullingParamAlloc.isValid = true;


        GPUBufferAllocator* currIndirectDrawArgsBuffer = GetCurrentCullingParamBuffer(frameID);
        currIndirectDrawArgsBuffer = GetCurrentIndirectDrawArgsBuffer(frameID);
        BufferAllocation indirectDrawArgsAlloc;
        indirectDrawArgsAlloc.buffer = currIndirectDrawArgsBuffer->GetGPUBuffer();
        indirectDrawArgsAlloc.offset = 0;
        indirectDrawArgsAlloc.gpuAddress = currIndirectDrawArgsBuffer->GetBaseGPUAddress();
        indirectDrawArgsAlloc.isValid = true;

        // Get Current BatchInfo:
        vector<IndirectDrawSource> batchInfo = BatchManager::GetInstance()->GetBatchInfo();
        if (batchInfo.size() != 0) 
        {
            currIndirectDrawArgsBuffer->UploadBufferStaged(indirectDrawArgsAlloc, batchInfo.data(), batchInfo.size() * sizeof(IndirectDrawSource));
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

        
        // GPUScenePass:

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
        
        Payload_DispatchComputeShader payload = {};
        payload.csShader = RenderEngine::GetInstance()->GetGPUScene().GetCullingShaderHandler().Get();
        payload.groupX = gameObjectCount / 64 + 1;
        payload.groupY = 1;
        payload.groupZ = 1;
        RenderBackend::GetInstance()->DispatchComputeShader(payload);

        RenderBackend::GetInstance()->UAVBarrier(visibilityBuffer);
        RenderBackend::GetInstance()->UAVBarrier(currIndirectDrawArgsBuffer->GetGPUBuffer());

        IGPUBuffer* currIndirectDrawCountBuffer = indirectDrawCountBuffers->GetCurrentBufferByFrameID(frameID)->GetGPUBuffer();
        IGPUBuffer* currIndirectDrawDestBuffer = indirectDrawDestBuffers->GetCurrentBufferByFrameID(frameID)->GetGPUBuffer();
        vector<uint32_t> buffer{ 0, 3000 };
        BufferAllocation alloc;
        alloc.isValid = true;
        alloc.offset = 0;
        alloc.buffer = indirectDrawCountBuffers->GetCurrentBufferByFrameID(frameID)->GetGPUBuffer();
        indirectDrawCountBuffers->GetCurrentBufferByFrameID(frameID)->UploadBufferStaged(alloc, buffer.data(), buffer.size() * sizeof(uint32_t));

        RenderBackend::GetInstance()->SetResourceState(currIndirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);
        RenderBackend::GetInstance()->SetResourceState(indirectDrawCountBuffers->GetCurrentBufferByFrameID(frameID)->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);


        RenderBackend::GetInstance()->SetResourceState(currIndirectDrawDestBuffer, BufferResourceState::STATE_UNORDERED_ACCESS);
        RenderBackend::GetInstance()->SetResourceState(currIndirectDrawCountBuffer, BufferResourceState::STATE_UNORDERED_ACCESS);

        //---------------- IndirectDrawCall Combine Pass
        ComputeShader* combineCSShader = IndirectDrawCombineShaderHandler.Get();
        combineCSShader->SetBuffer("IndirectDrawSourceBuffer", currIndirectDrawArgsBuffer->GetGPUBuffer());
        combineCSShader->SetBuffer("IndirectDrawDestBuffer", currIndirectDrawDestBuffer);
        combineCSShader->SetBuffer("IndirectDrawCount", currIndirectDrawCountBuffer);
        
        BufferAllocation allocation;
        allocation.isValid = true;
        allocation.offset = 0;
        allocation.size = sizeof(uint32_t);

        uint32_t batchCount = BatchManager::GetInstance()->GetBatchInfo().size();
        batchCounterBuffers->GetCurrentBufferByFrameID(frameID)->UploadBuffer(allocation, &batchCount, sizeof(uint32_t));
        combineCSShader->SetBuffer("BatchCountBuffer", batchCounterBuffers->GetCurrentBufferByFrameID(frameID)->GetGPUBuffer());
        
        Payload_DispatchComputeShader combinePayload = {};
        combinePayload.csShader = combineCSShader;
        combinePayload.groupX = batchCount / 64 + 1;
        combinePayload.groupY = 1;
        combinePayload.groupZ = 1;
        RenderBackend::GetInstance()->DispatchComputeShader(combinePayload);
        RenderBackend::GetInstance()->UAVBarrier(currIndirectDrawDestBuffer);
        RenderBackend::GetInstance()->UAVBarrier(currIndirectDrawCountBuffer);

        // 这个地方简单跑个绑定测试？
        RenderBackend::GetInstance()->SetResourceState(visibilityBuffer, BufferResourceState::STATE_SHADER_RESOURCE);
        RenderBackend::GetInstance()->SetResourceState(currIndirectDrawDestBuffer, BufferResourceState::STATE_INDIRECT_ARGUMENT);
        RenderBackend::GetInstance()->SetResourceState(currIndirectDrawCountBuffer, BufferResourceState::STATE_INDIRECT_ARGUMENT);

        context.IndirectDrawArgsBuffer = currIndirectDrawDestBuffer;
        context.IndirectDrawCountCountBuffer = currIndirectDrawCountBuffer;
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
        uint32_t frameID = RenderEngine::GetInstance()->GetCurrentFrame();
        //PROFILER_ZONE("MainThread::Renderer::Render")
        for(auto* pass : context.camera->mRenderPassAsset.renderPasses)
        {
            pass->Execute(context);
            RenderBackend::GetInstance()->TryWakeUpRenderThread();
        }

       // //---------------- IndirectDrawCall Combine Pass
       // RenderTexture* currHIZTexture = RWHiZTexture[frameID % 3];
       // ComputeShader* blitShader = BlitCSShaderHandler.Get();
       // Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->GetMainCamera();
       // ASSERT(cam);
       // RenderBackend::GetInstance()->SetResourceState(cam->depthAttachment->textureBuffer, //BufferResourceState::STATE_DEPTH_READ_SHADER_RESOURCE);
       // blitShader->SetTexture("SrcTexture", cam->depthAttachment->textureBuffer, 0);
       // blitShader->SetTexture("DstTexture", currHIZTexture->textureBuffer, 0);
       // Payload_DispatchComputeShader blitPayload = {};
       // blitPayload.csShader = blitShader;
       // blitPayload.groupX = 1920 / 8;
       // blitPayload.groupY = 1080 / 8;
       // blitPayload.groupZ = 1;
       // RenderBackend::GetInstance()->DispatchComputeShader(blitPayload);
       // RenderBackend::GetInstance()->SetResourceState(cam->depthAttachment->textureBuffer, //BufferResourceState::STATE_DEPTH_WRITE);
       // //ComputeShader* hiZCSShader = HizCSShaderHandler.Get();
        //// 1. blit to Src...
        //for (int i = 0; i < currHIZTexture->GetMipCount() - 1; i++)
        //{
        //    hiZCSShader->SetTexture("g_InputTexture", currHIZTexture->textureBuffer, i);
        //    hiZCSShader->SetTexture("g_OutputTexture", currHIZTexture->textureBuffer, i + 1);
        //    RenderBackend::GetInstance()->UAVBarrier(currHIZTexture->textureBuffer);
        //}
        
        
        for (auto& pass : context.camera->mRenderPassAsset.renderPasses)
        {
            pass->Clear();
        }
    }

    GPUBufferAllocator* GPUSceneRenderPipeline::GetCurrentCullingParamBuffer(uint32_t frameID)
    {
        return cullingParamBuffers->GetCurrentBufferByFrameID(frameID);
    } 

    GPUBufferAllocator* GPUSceneRenderPipeline::GetCurrentIndirectDrawArgsBuffer(uint32_t frameID)
    {
        return indirectDrawSourceBuffers->GetCurrentBufferByFrameID(frameID);
    }
}
