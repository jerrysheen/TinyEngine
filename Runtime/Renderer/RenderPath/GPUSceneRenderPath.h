#pragma once
#include "IRenderPath.h"
#include "Graphics/GPUSceneManager.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderCommand.h"
#include "Graphics/GPUBufferAllocator.h"
#include "Graphics/IGPUResource.h"
#include "Math/Frustum.h"
#include "Renderer/BatchManager.h"


namespace EngineCore
{
    class GPUSceneRenderPath : public IRenderPath
    {
    public:
        virtual ~GPUSceneRenderPath() override 
        {
            delete cullingParamBuffer;
            delete indirectDrawArgsBuffer;
        };

        virtual void Execute(RenderContext& context) override
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
                desc.size = sizeof(DrawIndirectArgs) * 100;
                desc.stride = sizeof(DrawIndirectArgs);
                desc.usage = BufferUsage::StructuredBuffer;
                indirectDrawArgsBuffer = new GPUBufferAllocator(desc);
                indirectDrawArgsAlloc = indirectDrawArgsBuffer->Allocate(sizeof(DrawIndirectArgs) * 100);
            }

            //todo:
            // 这个地方要把ResourceState切换一下
            Renderer::GetInstance()->BeginFrame();
            auto* visibilityBuffer = GPUSceneManager::GetInstance()->visibilityBuffer;
            Renderer::GetInstance()->SetResourceState(visibilityBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);
            Renderer::GetInstance()->SetResourceState(indirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);


            Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
            int gameObjectCount = SceneManager::GetInstance()->GetCurrentScene()->allObjList.size();

            GPUCullingParam cullingParam;
            cullingParam.frustum = cam->mFrustum;
            cullingParam.totalItem = gameObjectCount;
            cullingParamBuffer->UploadBuffer(cullingParamAlloc, &cullingParam, sizeof(GPUCullingParam));

            PROFILER_EVENT_BEGIN("MainThread::GPUSceneManagerTick");
            GPUSceneManager::GetInstance()->Tick();
            PROFILER_EVENT_END("MainThread::GPUSceneManagerTick");

            // Get Current BatchInfo:
            vector<DrawIndirectArgs> batchInfo = BatchManager::GetInstance()->GetBatchInfo();
            indirectDrawArgsBuffer->UploadBuffer(indirectDrawArgsAlloc, batchInfo.data(), batchInfo.size() * sizeof(DrawIndirectArgs));

            ComputeShader* csShader = GPUSceneManager::GetInstance()->GPUCullingShaderHandler.Get();
            csShader->SetBuffer("g_InputAABBs", GPUSceneManager::GetInstance()->allAABBBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_InputPerObjectDatas", GPUSceneManager::GetInstance()->allObjectDataBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_RenderProxies", GPUSceneManager::GetInstance()->renderProxyBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_VisibleInstanceIndices", GPUSceneManager::GetInstance()->visibilityBuffer->GetGPUBuffer());
            csShader->SetBuffer("CullingParams", cullingParamBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_IndirectDrawCallArgs", indirectDrawArgsBuffer->GetGPUBuffer());

            
            Payload_DispatchComputeShader payload;
            payload.csShader = GPUSceneManager::GetInstance()->GPUCullingShaderHandler.Get();
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

        bool hasSetUpBuffer = false;
        BufferAllocation cullingParamAlloc;
        GPUBufferAllocator* cullingParamBuffer;
        BufferAllocation indirectDrawArgsAlloc;
        GPUBufferAllocator* indirectDrawArgsBuffer;

    };
}