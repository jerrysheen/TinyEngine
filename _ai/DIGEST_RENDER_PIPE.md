# Architecture Digest: RENDER_PIPE
> Auto-generated. Focus: Runtime/Renderer, Runtime/Renderer/RenderPipeLine, Runtime/Renderer/RenderPath, RenderPipeLine, RenderPass, RenderPath, RenderContext, Batch, Culling, RenderEngine, Sort, Blit

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 渲染管线负责Pass组织、排序、裁剪与FrameGraph思路。
- 强调RenderContext、RenderPass与RenderPath的接口。

## Key Files Index
- `[70]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp** *(Content Included)*
- `[67]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp** *(Content Included)*
- `[67]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp** *(Content Included)*
- `[63]` **Runtime/Renderer/RenderPipeLine/RenderPass.h** *(Content Included)*
- `[62]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h** *(Content Included)*
- `[62]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h** *(Content Included)*
- `[61]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h** *(Content Included)*
- `[60]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[58]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h** *(Content Included)*
- `[52]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h** *(Content Included)*
- `[51]` **Runtime/Renderer/RenderPipeLine/RenderPass.cpp** *(Content Included)*
- `[47]` **Runtime/Renderer/RenderContext.cpp** *(Content Included)*
- `[46]` **Runtime/Renderer/RenderPath/IRenderPath.h** *(Content Included)*
- `[45]` **Runtime/Renderer/RenderEngine.h** *(Content Included)*
- `[42]` **Runtime/Renderer/RenderContext.h** *(Content Included)*
- `[41]` **Runtime/Renderer/BatchManager.cpp** *(Content Included)*
- `[39]` **Runtime/Renderer/RenderSorter.h** *(Content Included)*
- `[38]` **Runtime/Renderer/BatchManager.h** *(Content Included)*
- `[35]` **Runtime/Renderer/Culling.h** *(Content Included)*
- `[34]` **Runtime/Renderer/Culling.cpp** *(Content Included)*
- `[30]` **Assets/Shader/GPUCulling.hlsl**
- `[25]` **Assets/Shader/BlitShader.hlsl**
- `[23]` **Runtime/Renderer/Renderer.h**
- `[20]` **Runtime/Entry.cpp**
- `[19]` **Runtime/Renderer/Renderer.cpp**
- `[18]` **Runtime/Core/Game.cpp**
- `[18]` **Runtime/Graphics/GPUSceneManager.cpp**
- `[18]` **Runtime/Renderer/RenderAPI.h**
- `[17]` **Runtime/Renderer/RenderStruct.h**
- `[14]` **Runtime/Core/PublicStruct.h**
- `[12]` **Runtime/GameObject/Camera.cpp**
- `[12]` **Runtime/Renderer/PerDrawAllocator.h**
- `[12]` **Runtime/Renderer/RenderCommand.h**
- `[12]` **Runtime/Renderer/RenderUniforms.h**
- `[12]` **Runtime/Renderer/SPSCRingBuffer.h**
- `[12]` **Runtime/Scene/SceneManager.cpp**
- `[10]` **Runtime/Renderer/RenderAPI.cpp**
- `[9]` **Runtime/Graphics/GPUSceneManager.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[5]` **Runtime/GameObject/MeshFilter.cpp**
- `[5]` **Runtime/GameObject/MeshRenderer.cpp**
- `[5]` **Runtime/Settings/ProjectSettings.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12RootSignature.cpp**
- `[4]` **Runtime/GameObject/Camera.h**
- `[4]` **Runtime/Scene/SceneManager.h**
- `[4]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[4]` **Runtime/Platforms/D3D12/d3dUtil.cpp**
- `[4]` **Assets/Shader/SimpleTestShader.hlsl**
- `[4]` **Assets/Shader/StandardPBR.hlsl**
- `[4]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[3]` **Runtime/EngineCore.h**
- `[3]` **Runtime/Core/PublicEnum.h**
- `[3]` **Runtime/GameObject/MeshRenderer.h**
- `[3]` **Runtime/Graphics/GPUBufferAllocator.cpp**
- `[3]` **Runtime/Scene/BistroSceneLoader.cpp**
- `[3]` **Runtime/Settings/ProjectSettings.cpp**
- `[2]` **Editor/EditorGUIManager.h**
- `[2]` **Editor/EditorSettings.h**
- `[2]` **Runtime/CoreAssert.h**

## Evidence & Implementation Details

### File: `Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp`
```cpp
        m_LastMatState.Reset();

        Renderer::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        Renderer::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        Renderer::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);

        Renderer::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        if(!RenderSettings::s_EnableVertexPulling)
        {
            for(auto& [hashID, renderContext] : BatchManager::GetInstance()->drawIndirectContextMap)
            {
                int batchID = BatchManager::GetInstance()->drawIndirectParamMap[hashID].indexInDrawIndirectList;
                int stratIndex = BatchManager::GetInstance()->drawIndirectParamMap[hashID].startIndexInInstanceDataList;
                Material* mat = renderContext.material;
                Mesh* mesh = renderContext.mesh;
                // 根据mat + pass信息组织pippeline
                Renderer::GetInstance()->SetRenderState(mat, mRenderPassInfo);
                // copy gpu material data desc 
                Renderer::GetInstance()->SetBindlessMat(mat);
                // bind mesh vertexbuffer and indexbuffer.
                Renderer::GetInstance()->SetMeshData(mesh);
                Payload_DrawIndirect indirectPayload;
                // temp:
                GPUBufferAllocator* indirectDrawArgsBuffer = RenderEngine::gpuSceneRenderPath.indirectDrawArgsBuffer;
                ASSERT(indirectDrawArgsBuffer != nullptr);
                indirectPayload.indirectArgsBuffer = indirectDrawArgsBuffer->GetGPUBuffer();
                indirectPayload.count = 1;
                indirectPayload.startIndex = batchID;
                indirectPayload.startIndexInInstanceDataBuffer = stratIndex;
                Renderer::GetInstance()->DrawIndirect(indirectPayload);
            }
        }
```
...
```cpp
                {
                    m_LastMatState = mat->GetMaterialRenderState();
                    Renderer::GetInstance()->SetRenderState(mat, mRenderPassInfo);
                    Renderer::GetInstance()->SetBindlessMat(mat);
                    Renderer::GetInstance()->SetBindLessMeshIB(0);
                }

                Payload_DrawIndirect indirectPayload;
                // temp:
                GPUBufferAllocator* indirectDrawArgsBuffer = RenderEngine::gpuSceneRenderPath.indirectDrawArgsBuffer;
                ASSERT(indirectDrawArgsBuffer != nullptr);
                indirectPayload.indirectArgsBuffer = indirectDrawArgsBuffer->GetGPUBuffer();
                indirectPayload.count = 1;
                indirectPayload.startIndex = batchID;
                indirectPayload.startIndexInInstanceDataBuffer = stratIndex;
                Renderer::GetInstance()->DrawIndirect(indirectPayload);
            }
        }

    }

    void GPUSceneRenderPass::Filter(const RenderContext &context)
    {
```

### File: `Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp`
```cpp
        mRenderPassInfo.drawRecordList.emplace_back(mat, model);
        // todo： 后面挪到别的地方， 先做Batch的部分：
        Renderer::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        Renderer::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        Renderer::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        
        Renderer::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        
        // 根据mat + pass信息组织pippeline
        Renderer::GetInstance()->SetRenderState(mat, mRenderPassInfo);
        // copy gpu material data desc 
        Renderer::GetInstance()->SetMaterialData(mat);
        //Texture* tex = context.camera->colorAttachment.Get();
        //Renderer::GetInstance()->SetResourceState(tex, BufferResourceState::STATE_SHADER_RESOURCE);
        //Renderer::GetInstance()->BindTexture(tex, 0, 0);
        // bind mesh vertexbuffer and indexbuffer.
        Renderer::GetInstance()->SetMeshData(model);
        Renderer::GetInstance()->DrawIndexedInstanced(model, 1, PerDrawHandle{0,0});
```

### File: `Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp`
```cpp
                                        mRenderPassInfo.renderBatchList);

        PROFILER_EVENT_BEGIN("MainThread::OpaqueRenderPass::SetDrawCall");

        // todo： 后面挪到别的地方， 先做Batch的部分：
        Renderer::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        Renderer::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        Renderer::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);

        Renderer::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        for each(auto& record in mRenderPassInfo.renderBatchList)
        {
            // 根据mat + pass信息组织pippeline
            Renderer::GetInstance()->SetRenderState(record.mat, mRenderPassInfo);
            // copy gpu material data desc 
            Renderer::GetInstance()->SetMaterialData(record.mat);
            // bind mesh vertexbuffer and indexbuffer.
            Renderer::GetInstance()->SetMeshData(record.mesh);
            Renderer::GetInstance()->DrawIndexedInstanced(record.mesh, record.instanceCount, PerDrawHandle{ nullptr, (uint32_t)record.alloc.offset, 0 });
        }
```

### File: `Runtime/Renderer/RenderPipeLine/RenderPass.h`
```cpp
namespace EngineCore
{
    class RenderPass
    {
    public:

        // init RenderPass, for example Create FBO..
        //virtual void Create(const RenderContext& context) = 0;
        virtual void Create() = 0;
        
        // Record Rendertarget in mRenderPassInfo..
        virtual void Configure(const RenderContext& context) = 0;
        
        // decide what content we need to draw, for example
        // in opaquepass we only need gameobject renderqueue less than 3000.
        virtual void Filter(const RenderContext& context) = 0;
        
        // first record drawInfo then sync data to RenderAPI, finally Submit to execute the
        // real Draw.
        // todo :  这个地方需要修改， 因为现在里面需要增加sortingkey，所以没const，但是感觉有问题
        virtual void Execute(RenderContext& context) = 0;
        
        // sent current Data to Renderer-> RenderPassInfo
        virtual void Submit() = 0;

        inline void Clear() { mRenderPassInfo.Reset(); };


        // ignore null ptr since RenderAPI level will handle this problem
        inline void SetRenderTarget(RenderTexture* colorAttachment, RenderTexture* depthAttachment)
        {
            mRenderPassInfo.colorAttachment = colorAttachment;
            mRenderPassInfo.depthAttachment = depthAttachment;
        };

        inline void SetClearFlag(ClearFlag flag, const Vector3& colorValue = Vector3::Zero, float depthValue = 1.0)
        {
            mRenderPassInfo.clearFlag = flag;
            mRenderPassInfo.clearColorValue = colorValue;
            mRenderPassInfo.clearDepthValue = depthValue;
        };

        inline void SetViewPort(const Vector2& startPos, const Vector2& endPos)
        {
            mRenderPassInfo.viewportStartPos = startPos;
            mRenderPassInfo.viewportEndPos = endPos;
        }

        inline const RenderPassInfo& GetRenderPassInfo(){return mRenderPassInfo;};
        RootSigSlot mRootSigSlot;
    public:
        string name;
        RenderPassInfo mRenderPassInfo;
    };
} // namespace EngineCore
```

### File: `Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h`
```cpp
namespace EngineCore
{
    class GPUSceneRenderPass : public RenderPass
    {
    public:

        GPUSceneRenderPass();
        // renderpass 初始化
        //virtual void Create(const RenderContext& context) override;
        // todo Create 也需要参数
        virtual void Create() override;
        // 配置rt等
        virtual void Configure(const RenderContext& context) override;
        // 执行具体的draw
        virtual void Execute(RenderContext& context) override;

        virtual void Filter(const RenderContext& context) override;

        virtual void Submit() override;
    private:
        MaterailRenderState m_LastMatState;
    };
```

### File: `Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h`
```cpp
namespace EngineCore
{
    class OpaqueRenderPass : public RenderPass
    {
    public:

        OpaqueRenderPass();
        // renderpass 初始化
        //virtual void Create(const RenderContext& context) override;
        // todo Create 也需要参数
        virtual void Create() override;
        // 配置rt等
        virtual void Configure(const RenderContext& context) override;
        // 执行具体的draw
        virtual void Execute(RenderContext& context) override;

        virtual void Filter(const RenderContext& context) override;

        virtual void Submit() override;
    };
```

### File: `Runtime/Renderer/RenderPipeLine/FinalBlitPass.h`
```cpp
namespace EngineCore
{
    class FinalBlitPass : public RenderPass
    {
        public:
        // renderpass 初始化
        //virtual void Create(const RenderContext& context) override;
        // todo Create 也需要参数
        virtual void Create() override;
        // 配置rt等
        virtual void Configure(const RenderContext& context) override;
        // 执行具体的draw
        virtual void Execute(RenderContext& context) override;

        virtual void Filter(const RenderContext& context) override;

        virtual void Submit() override;

    };
```

### File: `Runtime/Renderer/RenderPath/GPUSceneRenderPath.h`
```cpp
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
                desc.size = sizeof(DrawIndirectArgs) * 3000;
                desc.stride = sizeof(DrawIndirectArgs);
                desc.usage = BufferUsage::StructuredBuffer;
                indirectDrawArgsBuffer = new GPUBufferAllocator(desc);
                indirectDrawArgsAlloc = indirectDrawArgsBuffer->Allocate(sizeof(DrawIndirectArgs) * 3000);
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
            if (batchInfo.size() != 0) 
            {
                indirectDrawArgsBuffer->UploadBuffer(indirectDrawArgsAlloc, batchInfo.data(), batchInfo.size() * sizeof(DrawIndirectArgs));
            }

            ComputeShader* csShader = GPUSceneManager::GetInstance()->GPUCullingShaderHandler.Get();
            csShader->SetBuffer("CullingParams", cullingParamBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_InputAABBs", GPUSceneManager::GetInstance()->allAABBBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_InputPerObjectDatas", GPUSceneManager::GetInstance()->allObjectDataBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_RenderProxies", GPUSceneManager::GetInstance()->renderProxyBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_VisibleInstanceIndices", GPUSceneManager::GetInstance()->visibilityBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_IndirectDrawCallArgs", indirectDrawArgsBuffer->GetGPUBuffer());

            
            Payload_DispatchComputeShader payload;
            payload.csShader = GPUSceneManager::GetInstance()->GPUCullingShaderHandler.Get();
            payload.groupX = gameObjectCount / 64 + 1;
            payload.groupY = 1;
            payload.groupZ = 1;
            Renderer::GetInstance()->DispatchComputeShader(payload);
```
...
```cpp

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
```

### File: `Runtime/Renderer/RenderPath/LagacyRenderPath.h`
```cpp
namespace EngineCore
{
    class LagacyRenderPath : public IRenderPath
    {
    public:
        virtual ~LagacyRenderPath() override {};
        virtual void Execute(RenderContext& context) override
        {
            Renderer::GetInstance()->BeginFrame();
            
            PROFILER_EVENT_BEGIN("MainThread::GPUSceneManagerTick");
            GPUSceneManager::GetInstance()->Tick();
            PROFILER_EVENT_END("MainThread::GPUSceneManagerTick");

            context.Reset();

            // todo： 这个地方culling逻辑是不是应该放到Update
            Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
            context.camera = cam;
            PROFILER_EVENT_BEGIN("MainThread::Culling::Run");
            Culling::Run(cam, context);
            PROFILER_EVENT_END("MainThread::Culling::Run");

            Renderer::GetInstance()->Render(context);

#ifdef EDITOR
            Renderer::GetInstance()->OnDrawGUI();
#endif

            Renderer::GetInstance()->EndFrame();
        }
    };
```

### File: `Runtime/Renderer/RenderPath/IRenderPath.h`
```cpp
{
    class Renderer;
    class IRenderPath
    {
    public:
        virtual ~IRenderPath() = default;
        virtual void Execute(RenderContext& context) = 0;
    };
```

### File: `Runtime/Renderer/RenderEngine.h`
```cpp
    // render: 执行场景渲染，不同管线进行渲染等。
    // end render : RenderAPI->EndFrame() 调用Present，buckbufferIndex Update
    class RenderEngine
    {
    public:
        static RenderEngine* GetInstance(){return s_Instance.get();};
        static bool IsInitialized(){return s_Instance != nullptr;};
        static void Update();
        static void Create();
        
        static void OnResize(int width, int height);
        static void OnDrawGUI();
        static void Tick();

        static void Destory();
        RenderEngine(){};
        ~RenderEngine(){};
        static void WaitForLastFrameFinished();
        static void SignalMainThreadSubmited();
        static GPUSceneRenderPath gpuSceneRenderPath;
    private:
        static std::unique_ptr<RenderEngine> s_Instance;
        static RenderContext renderContext;
        static LagacyRenderPath lagacyRenderPath;
    };
```

### File: `Runtime/Renderer/RenderContext.h`
```cpp
{
    // 记录灯光，阴影，物体信息？
    class RenderContext
    {
    public:
        RenderContext()
        {
            visibleItems.resize(10000);
        }
        Camera* camera;
        vector<LightData*> visibleLights;
        vector<RenderPacket> visibleItems;
        int currentMaxIndex = 0;

        inline void Reset()
        {
            camera = nullptr;
            visibleLights.clear();
            visibleItems.clear();
        }

        static void DrawRenderers(RenderContext& renderContext, 
                           const ContextDrawSettings& drawingSettings, 
                           const ContextFilterSettings& filteringSettings, 
                           std::vector<RenderBatch>& outDrawRecords);
        
        static void BatchContext(std::vector<RenderPacket*>& sortedItem);

        static bool CanBatch(const RenderBatch& batch, const RenderPacket item);

    private:

    };
```

### File: `Runtime/Renderer/RenderSorter.h`
```cpp
    }    

    class RendererSort
    {
    public:
        static void BuildSortKeys(const RenderContext& context, std::vector<RenderPacket>& items,
                                SortingCriteria sortingCriteria)
        {
            Vector3 camPos = context.camera->gameObject->transform->GetWorldPosition();
            float nearPlane = context.camera->mNear;
            float farPlane = context.camera->mFar;
            for(auto& item : items)
            {
                uint32_t meshID = item.meshFilter->mMeshHandle->GetAssetID();
                uint32_t matID = item.meshRenderer->GetMaterial()->GetAssetID();

                float distance = item.distanToCamera;
                uint16_t distanceToID = FloatToDepth(distance, nearPlane, farPlane);
                switch(sortingCriteria)
                {
                    case SortingCriteria::ComonOpaque:
                    break;
                    case SortingCriteria::ComonTransparent:
                        distanceToID = 0xFFFF - distanceToID;
                    break;
                }
                // todo : 用游戏isntanceID，改成24位足够了
                uint64_t key = 0;
                key |= (uint64_t)(0) << 60; // Layer 0
                key |= (uint64_t)(matID & 0x0FFFFFFF) << 32; 
                key |= (uint64_t)(meshID & 0xFFFF) << 16;
                key |= (uint64_t)(distanceToID & 0xFFFF); 
                item.sortingKey = key;
            }
        }
    };
```

### File: `Runtime/Renderer/BatchManager.h`
```cpp

    class MeshRenderer;
    class BatchManager
    {
    public:
        static BatchManager* GetInstance()
        {
            if(s_Instance == nullptr)
            {
                Create();
            }
            return s_Instance;
        }

        void TryAddBatchCount(MeshRenderer* meshRenderer);
        void TryDecreaseBatchCount(MeshRenderer* meshRenderer);

        void TryAddBatchCount(MeshFilter* meshFilter);
        void TryDecreaseBatchCount(MeshFilter* meshFilter);
        static std::unordered_map<uint64_t, int> BatchMap;
        static std::unordered_map<uint64_t, DrawIndirectParam> drawIndirectParamMap;
        static std::unordered_map<uint64_t, DrawIndirectContext> drawIndirectContextMap;

        std::vector<RenderProxy> GetAvaliableRenderProxyList(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        static uint64_t GetBatchHash(MeshRenderer* meshRenderer, MeshFilter* meshFilter, uint32_t layer); 

        vector<DrawIndirectArgs> GetBatchInfo();
    private:
        void TryAddBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        void TryDecreaseBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        static void Create();
        static BatchManager* s_Instance;

    };
```

### File: `Runtime/Renderer/Culling.h`
```cpp
namespace EngineCore
{
    class Culling
    {
    public:
        static void Run(Camera* cam, RenderContext& context);
        
    };
```