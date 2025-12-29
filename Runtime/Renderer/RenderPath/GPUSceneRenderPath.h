#pragma once
#include "IRenderPath.h"
#include "Graphics/GPUSceneManager.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderCommand.h"
#include "Graphics/GPUBufferAllocator.h"
#include "Graphics/IGPUBuffer.h"
#include "Math/Frustum.h"
#include "Renderer/BatchManager.h"


namespace EngineCore
{
    class GPUSceneRenderPath : public IRenderPath
    {
    public:
        virtual ~GPUSceneRenderPath() override 
        {
            delete visibilityBuffer;
            delete visibilityCounterBuffer;
            delete cullingParamBuffer;
            delete indirectDrawArgsBuffer;
        };

        virtual void Execute(RenderContext& context) override
        {
            if (!hasSetUpBuffer) 
            {
                hasSetUpBuffer = true;

                BufferDesc desc;
                desc.debugName = L"VisibilityBuffer";
                desc.memoryType = BufferMemoryType::Default;
                desc.size = 4 * 10000;
                desc.stride = 4 * 10000;
                desc.usage = BufferUsage::ByteAddressBuffer;
                visibilityBuffer = new GPUBufferAllocator(desc);
                visiblityAlloc = visibilityBuffer->Allocate(4 * 10000);

                desc.debugName = L"VisibilityCounter";
                desc.memoryType = BufferMemoryType::Default;
                desc.size = 256;
                desc.stride = 256;
                desc.usage = BufferUsage::ByteAddressBuffer;
                visibilityCounterBuffer = new GPUBufferAllocator(desc);
                visiblityCounterAlloc = visibilityCounterBuffer->Allocate(256);

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
            vector<uint8_t> data;
            data.resize(4, 0);
            visibilityCounterBuffer->UploadBuffer(visiblityCounterAlloc, data.data(), data.size());
            data.resize(4 * 10000, 0);
            visibilityBuffer->UploadBuffer(visiblityAlloc, data.data(), data.size());

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

            context.Reset();

            ComputeShader* csShader = GPUSceneManager::GetInstance()->GPUCullingShaderHandler.Get();
            csShader->SetBuffer("g_InputPerObjectDatas", GPUSceneManager::GetInstance()->allObjectDataBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_RenderProxies", GPUSceneManager::GetInstance()->renderProxyBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_VisibleInstanceIndices", visibilityBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_CounterBuffer", visibilityCounterBuffer->GetGPUBuffer());
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
                        // 这个地方简单跑个绑定测试？
            RenderAPI::GetInstance()->SetResourceState();
            Renderer::GetInstance()->DrawIndirect();            // 这个地方简单跑个绑定测试？
            RenderAPI::GetInstance()->SetResourceState();
            Renderer::GetInstance()->DrawIndirect();

#ifdef EDITOR
            Renderer::GetInstance()->OnDrawGUI();
#endif

            Renderer::GetInstance()->EndFrame();
        }

        bool hasSetUpBuffer = false;
        BufferAllocation visiblityAlloc;
        GPUBufferAllocator* visibilityBuffer;
        BufferAllocation visiblityCounterAlloc;
        GPUBufferAllocator* visibilityCounterBuffer;
        BufferAllocation cullingParamAlloc;
        GPUBufferAllocator* cullingParamBuffer;
        BufferAllocation indirectDrawArgsAlloc;
        GPUBufferAllocator* indirectDrawArgsBuffer;

    };
}