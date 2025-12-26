#pragma once
#include "IRenderPath.h"
#include "Graphics/GPUSceneManager.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderCommand.h"
#include "Graphics/PersistantBuffer.h"
#include "Graphics/IGPUBuffer.h"
#include "Math/Frustum.h"

namespace EngineCore
{
    class GPUSceneRenderPath : public IRenderPath
    {
    public:
        virtual ~GPUSceneRenderPath() override {};
        virtual void Execute(RenderContext& context) override
        {
            if (!hasSetUpBuffer) 
            {
                hasSetUpBuffer = true;

                BufferDesc desc;
                desc.debugName = L"VisibilityBuffer";
                desc.memoryType = BufferMemoryType::Default;
                desc.size = 4 * 10000;
                desc.stride = 1;
                desc.usage = BufferUsage::ByteAddressBuffer;
                visibilityBuffer = new PersistantBuffer(desc);

                desc.debugName = L"VisibilityCounter";
                desc.memoryType = BufferMemoryType::Default;
                desc.size = 256;
                desc.stride = 256;
                desc.usage = BufferUsage::ByteAddressBuffer;
                visibilityCounterBuffer = new PersistantBuffer(desc);
                visiblityAlloc = visibilityCounterBuffer->Allocate(256);

                desc.debugName = L"CullingParamBuffer";
                desc.memoryType = BufferMemoryType::Upload;
                desc.size = sizeof(Frustum);
                desc.stride = 1;
                desc.usage = BufferUsage::ConstantBuffer;
                cullingParamBuffer = new PersistantBuffer(desc);
            }

            Renderer::GetInstance()->BeginFrame();
            vector<uint8_t> data;
            data.resize(4, 0);
            visibilityCounterBuffer->UploadBuffer(visiblityAlloc, data.data(), data.size());

            PROFILER_EVENT_BEGIN("MainThread::GPUSceneManagerTick");
            GPUSceneManager::GetInstance()->Tick();
            PROFILER_EVENT_END("MainThread::GPUSceneManagerTick");

            context.Reset();

            ComputeShader* csShader = GPUSceneManager::GetInstance()->GPUCullingShaderHandler.Get();
            //csShader->SetBuffer("g_InputAABBs", GPUSceneManager::GetInstance()->allAABBBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_VisibleInstanceIndices", visibilityBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_CounterBuffer", visibilityCounterBuffer->GetGPUBuffer());
            //csShader->SetBuffer("CullingParams", cullingParamBuffer->GetGPUBuffer());

            Payload_DispatchComputeShader payload;
            payload.csShader = GPUSceneManager::GetInstance()->GPUCullingShaderHandler.Get();
            payload.groupX = 1;
            payload.groupY = 1;
            payload.groupZ = 1;
            Renderer::GetInstance()->DispatchComputeShader(payload);
            // 先把shader跑起来， 渲染到RT上， 后续blit啥的接入后面再说

#ifdef EDITOR
            Renderer::GetInstance()->OnDrawGUI();
#endif

            Renderer::GetInstance()->EndFrame();
        }

        bool hasSetUpBuffer = false;
        PersistantBuffer* visibilityBuffer;
        BufferAllocation visiblityAlloc;
        PersistantBuffer* visibilityCounterBuffer;
        PersistantBuffer* cullingParamBuffer;

    };
}