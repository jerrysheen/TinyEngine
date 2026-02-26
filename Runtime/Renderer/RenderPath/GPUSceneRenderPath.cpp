#include "PreCompiledHeader.h"
#include "GPUSceneRenderPath.h"
#include "Renderer/RenderEngine.h"

namespace EngineCore
{
    void GPUSceneRenderPath::Execute(RenderContext &context)
    {
        if (!hasSetUpBuffer) 
        {
            hasSetUpBuffer = true;

            BufferDesc desc;

            desc.debugName = L"CullingParamBuffer";
            desc.memoryType = BufferMemoryType::Upload;
            desc.size = sizeof(GPUCullingParam);
            desc.stride = sizeof(GPUCullingParam);
            desc.usage = BufferUsage::ConstantBuffer;
            cullingParamBuffer = new GPUBufferAllocator(desc);
            cullingParamAlloc = cullingParamBuffer->Allocate(sizeof(Frustum));
            
            
            desc.debugName = L"IndirectDrawArgsBuffer";
            desc.memoryType = BufferMemoryType::Default;
            desc.size = sizeof(DrawIndirectArgs) * 3000;
            desc.stride = sizeof(DrawIndirectArgs);
            desc.usage = BufferUsage::StructuredBuffer;
            indirectDrawArgsBuffer = new GPUBufferAllocator(desc);
            indirectDrawArgsAlloc = indirectDrawArgsBuffer->Allocate(sizeof(DrawIndirectArgs) * 3000);
        }

        //todo:
        // 这个地方要把ResourceState切换一下
        Renderer::GetInstance()->BeginFrame();

        FrameContext* currentFrame = RenderEngine::GetInstance()->GetGPUScene().GetCurrentFrameContexts();
        auto* visibilityBuffer = currentFrame->visibilityBuffer;
        Renderer::GetInstance()->SetResourceState(visibilityBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);
        Renderer::GetInstance()->SetResourceState(indirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);


        Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
        int gameObjectCount = SceneManager::GetInstance()->GetCurrentScene()->allObjList.size();

        GPUCullingParam cullingParam;
        cullingParam.frustum = cam->mFrustum;
        cullingParam.totalItem = gameObjectCount;
        cullingParamBuffer->UploadBuffer(cullingParamAlloc, &cullingParam, sizeof(GPUCullingParam));

        
        // Get Current BatchInfo:
        vector<DrawIndirectArgs> batchInfo = BatchManager::GetInstance()->GetBatchInfo();
        if (batchInfo.size() != 0) 
        {
            indirectDrawArgsBuffer->UploadBuffer(indirectDrawArgsAlloc, batchInfo.data(), batchInfo.size() * sizeof(DrawIndirectArgs));
        }

        ComputeShader* csShader = RenderEngine::GetInstance()->GetGPUScene().GetCullingShaderHandler().Get();
        csShader->SetBuffer("CullingParams", cullingParamBuffer->GetGPUBuffer());
        csShader->SetBuffer("g_InputAABBs", currentFrame->allAABBBuffer->GetGPUBuffer());
        csShader->SetBuffer("g_InputPerObjectDatas", currentFrame->allObjectDataBuffer->GetGPUBuffer());
        csShader->SetBuffer("g_RenderProxies", currentFrame->renderProxyBuffer->GetGPUBuffer());
        csShader->SetBuffer("g_VisibleInstanceIndices", currentFrame->visibilityBuffer->GetGPUBuffer());
        csShader->SetBuffer("g_IndirectDrawCallArgs", indirectDrawArgsBuffer->GetGPUBuffer());

        
        Payload_DispatchComputeShader payload;
        payload.csShader = RenderEngine::GetInstance()->GetGPUScene().GetCullingShaderHandler().Get();
        payload.groupX = gameObjectCount / 64 + 1;
        payload.groupY = 1;
        payload.groupZ = 1;
        Renderer::GetInstance()->DispatchComputeShader(payload);
        // 先把shader跑起来， 渲染到RT上， 后续blit啥的接入后面再说

        // 这个地方简单跑个绑定测试？
        Renderer::GetInstance()->SetResourceState(visibilityBuffer->GetGPUBuffer(), BufferResourceState::STATE_SHADER_RESOURCE);
        Renderer::GetInstance()->SetResourceState(indirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_INDIRECT_ARGUMENT);

        context.Reset();
        context.camera = cam;
        Renderer::GetInstance()->Render(context);
#ifdef EDITOR
        Renderer::GetInstance()->OnDrawGUI();
#endif

        Renderer::GetInstance()->EndFrame();
    }
}