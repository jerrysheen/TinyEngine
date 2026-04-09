# Architecture Digest: RENDER_PIPELINE
> Auto-generated. Focus: Runtime/Renderer, Runtime/Renderer/RenderPipeLine, Runtime/Renderer/RenderPath, RenderPipeLine, RenderPass, RenderPath, RenderContext, Batch, Culling, RenderEngine, Sort, Blit, GPUSceneRenderPath, LegacyRenderPath, Renderer, FrameContext, SetCurrentFrame, UpdateDirtyNode, ApplyDirtyNode, UploadCopyOp, FinalBlitPass, GPUSceneRenderPass, OpaqueRenderPass, EndFrame, BatchManager, GPUCulling, Indirect, DrawIndirectArgs, visibilityBuffer, CopyOp, PerObjectData, perFrameUploadBuffer, PersistentGPUScene, SceneDelta, DirtyFlags, RenderBuild, RenderSubmit, BuildVsSubmit, MainThreadFrontend, RenderThreadBackend

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作，并建立解耦的帧更新流（GameObject/Component、Scene、CPUScene/GPUScene、FrameContext多帧同步）。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。
- 针对更新链路重点追踪：Game::Update/Render/EndFrame -> SceneManager/Scene -> CPUScene -> GPUScene -> FrameContext。
- 重点识别NodeDirtyFlags、NodeDirtyPayload、PerFrameDirtyList、CopyOp等脏数据传播与跨帧同步结构。

## Understanding Notes
- 渲染管线负责Pass组织、排序、裁剪与FrameGraph思路。
- 强调RenderContext、RenderPass、RenderPath、FrameContext的接口，包含GPUCulling、BatchManager批处理、IndirectDraw间接绘制。

## Key Files Index
- `[121]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp** *(Content Included)*
- `[109]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp** *(Content Included)*
- `[108]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp** *(Content Included)*
- `[106]` **Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.cpp** *(Content Included)*
- `[95]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h** *(Content Included)*
- `[95]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h** *(Content Included)*
- `[93]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h** *(Content Included)*
- `[87]` **Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.h** *(Content Included)*
- `[82]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[80]` **Runtime/Renderer/BatchManager.h** *(Content Included)*
- `[77]` **Runtime/Renderer/BatchManager.cpp** *(Content Included)*
- `[76]` **Runtime/Renderer/RenderPath/LagacyRenderPipeline.cpp** *(Content Included)*
- `[76]` **Runtime/Renderer/RenderPipeLine/RenderPass.h** *(Content Included)*
- `[72]` **Runtime/Renderer/RenderPath/LagacyRenderPipeline.h** *(Content Included)*
- `[70]` **Runtime/Renderer/RenderPipeLine/RenderPass.cpp** *(Content Included)*
- `[69]` **Runtime/Renderer/RenderPath/IRenderPipeline.h** *(Content Included)*
- `[65]` **Runtime/Renderer/RenderEngine.h** *(Content Included)*
- `[64]` **Runtime/Renderer/RenderContext.cpp** *(Content Included)*
- `[61]` **Assets/Shader/GPUCulling.hlsl** *(Content Included)*
- `[56]` **Runtime/Renderer/RenderContext.h** *(Content Included)*
- `[50]` **Runtime/Renderer/RenderSorter.h**
- `[48]` **Runtime/Renderer/Culling.cpp**
- `[45]` **Runtime/Renderer/Culling.h**
- `[40]` **Runtime/Renderer/RenderBackend.h**
- `[37]` **Runtime/Renderer/RenderCommand.h**
- `[37]` **Runtime/Scene/GPUScene.cpp**
- `[36]` **Runtime/Renderer/RenderBackend.cpp**
- `[35]` **Runtime/Renderer/RenderStruct.h**
- `[34]` **Assets/Shader/IndirectDrawCallCombineComputeShader.hlsl**
- `[33]` **Runtime/Renderer/RenderAPI.h**
- `[30]` **Runtime/Core/Game.cpp**
- `[30]` **Runtime/Scene/GPUScene.h**
- `[27]` **Runtime/GameObject/MeshRenderer.cpp**
- `[27]` **Runtime/GameObject/MeshRenderer.h**
- `[25]` **Assets/Shader/BlitShader.hlsl**
- `[24]` **Runtime/Renderer/RenderUniforms.h**
- `[24]` **Runtime/Renderer/UploadPagePool.h**
- `[22]` **Runtime/Renderer/FrameTicket.h**
- `[22]` **Runtime/Renderer/PerDrawAllocator.h**
- `[22]` **Runtime/Renderer/SPSCRingBuffer.h**
- `[21]` **Runtime/GameObject/Camera.cpp**
- `[20]` **Runtime/Entry.cpp**
- `[20]` **Runtime/Renderer/FrameTicket.cpp**
- `[20]` **Runtime/Renderer/RenderAPI.cpp**
- `[20]` **Runtime/Renderer/UploadPagePool.cpp**
- `[19]` **Runtime/Scene/CPUScene.cpp**
- `[19]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[18]` **Runtime/Core/PublicStruct.h**
- `[16]` **Runtime/Scene/SceneManager.cpp**
- `[15]` **Runtime/Scene/Scene.cpp**
- `[13]` **Runtime/Scene/Scene.h**
- `[10]` **Runtime/Scene/SceneManager.h**
- `[8]` **Runtime/Graphics/Material.cpp**
- `[8]` **Assets/Shader/SimpleTestShader.hlsl**
- `[8]` **Assets/Shader/StandardPBR.hlsl**
- `[8]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[7]` **Runtime/Scene/CPUScene.h**
- `[7]` **Runtime/Serialization/SceneLoader.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[6]` **Runtime/Scene/BistroSceneLoader.cpp**

## Evidence & Implementation Details

### File: `Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp`
```cpp
            }

            std::sort(sortedKeys.begin(), sortedKeys.end());

            return sortedKeys;
        }
    }

    GPUSceneRenderPass::GPUSceneRenderPass()
    {
```
...
```cpp
        std::vector<uint64_t> sortedBatchKeys = BuildSortedBatchKeys(batchManager);

        RenderBackend::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        RenderBackend::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        RenderBackend::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);

        RenderBackend::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        if (!RenderSettings::s_EnableVertexPulling)
        {
            auto& indirectContextMap = batchManager->GetDrawIndirectContextMap();
            auto& indirectParamMap = batchManager->GetDrawIndirectParamMap();
            for (uint64_t hashID : sortedBatchKeys)
            {
                const DrawIndirectContext& renderContext = indirectContextMap.at(hashID);
                int batchID = indirectParamMap[hashID].indexInDrawIndirectList;
                int stratIndex = indirectParamMap[hashID].startIndexInInstanceDataList;
                Material* mat = renderContext.material;
                Mesh* mesh = renderContext.mesh;
                // 根据mat + pass信息组织pippeline
                RenderBackend::GetInstance()->SetRenderState(mat, mRenderPassInfo);
                // copy gpu material data desc 
                RenderBackend::GetInstance()->SetBindlessMat(mat);
                // bind mesh vertexbuffer and indexbuffer.
                RenderBackend::GetInstance()->SetMeshData(mesh);
                Payload_DrawIndirect indirectPayload;
                // temp:
                indirectPayload.indirectArgsBuffer = context.IndirectDrawArgsBuffer;
                indirectPayload.count = 1;
                indirectPayload.startIndex = batchID;
                indirectPayload.startIndexInInstanceDataBuffer = stratIndex;
                RenderBackend::GetInstance()->DrawIndirect(indirectPayload);
            }
        }
```
...
```cpp
                {
                    m_LastMatState = mat->GetMaterialRenderState();
                    RenderBackend::GetInstance()->SetRenderState(mat, mRenderPassInfo);
                    RenderBackend::GetInstance()->SetBindlessMat(mat);
                    RenderBackend::GetInstance()->SetBindLessMeshIB(0);
                }

                Payload_DrawIndirect indirectPayload;
                indirectPayload.indirectArgsBuffer = context.IndirectDrawArgsBuffer;
                indirectPayload.indirectDrawCountBuffer = context.IndirectDrawCountCountBuffer;
                indirectPayload.count = static_cast<uint32_t>(sortedBatchKeys.size());
                indirectPayload.startIndex = 0;
                indirectPayload.startIndexInInstanceDataBuffer = 0;
                RenderBackend::GetInstance()->DrawIndirect(indirectPayload);
            }
        }

    }

    void GPUSceneRenderPass::Filter(const RenderContext& context)
    {
```

### File: `Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp`
```cpp

        // todo： 后面挪到别的地方， 先做Batch的部分：
        RenderBackend::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        RenderBackend::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        RenderBackend::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        
        RenderBackend::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        
        // 根据mat + pass信息组织pippeline
        RenderBackend::GetInstance()->SetRenderState(mat, mRenderPassInfo);
        // copy gpu material data desc 
        RenderBackend::GetInstance()->SetMaterialData(mat);
        //Texture* tex = context.camera->colorAttachment.Get();
        //Renderer::GetInstance()->SetResourceState(tex, BufferResourceState::STATE_SHADER_RESOURCE);
        //Renderer::GetInstance()->BindTexture(tex, 0, 0);
        // bind mesh vertexbuffer and indexbuffer.
        RenderBackend::GetInstance()->SetMeshData(model);
        RenderBackend::GetInstance()->DrawIndexedInstanced(model, 1, PerDrawHandle{0,0});
```

### File: `Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp`
```cpp
    {

        PROFILER_EVENT_BEGIN("MainThread::OpaqueRenderPass::SetDrawCall");
        IssueRenderCommandCommon(mRenderPassInfo, mRenderPassInfo.renderBatchList);

        PROFILER_EVENT_END("MainThread::OpaqueRenderPass::SetDrawCall");

    }

    void OpaqueRenderPass::Filter(const RenderContext &context)
    {
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
        virtual void Prepare(RenderContext& context) override{};
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
        
        virtual void Prepare(RenderContext& context) override;


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
        virtual void Prepare(RenderContext& context) override;
        virtual void Execute(RenderContext& context) override;

        virtual void Filter(const RenderContext& context) override;

        virtual void Submit() override;

    private:
        Material* mat;
        Mesh* model;
    };
```

### File: `Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.h`
```cpp
namespace EngineCore
{
    class GPUSceneRenderPipeline : public IRenderPipeline
    {
    public:
        GPUSceneRenderPipeline();
        virtual ~GPUSceneRenderPipeline() override 
        {
            delete cullingParamBuffers;
            delete indirectDrawSourceBuffers;
        };
        virtual void Prepare(RenderContext& context) override;
        //virtual void Record(const CommandStream& cmdStream, RenderContext& context) override;
        virtual void RecordAndFlush(RenderContext& context) override;

        GPUBufferAllocator* GetCurrentCullingParamBuffer(uint32_t frameID);
        GPUBufferAllocator* GetCurrentIndirectDrawArgsBuffer(uint32_t frameID);

        PerFrameBufferRing* cullingParamBuffers;
        PerFrameBufferRing* indirectDrawSourceBuffers;
        PerFrameBufferRing* indirectDrawDestBuffers;
        PerFrameBufferRing* indirectDrawCountBuffers;
        PerFrameBufferRing* batchCounterBuffers;
        //BufferAllocation cullingParamAlloc;
        //GPUBufferAllocator* currCullingParamBuffer;
        //GPUBufferAllocator* cullingParamBuffer[3];

        //BufferAllocation indirectDrawArgsAlloc;
        //GPUBufferAllocator* currIndirectDrawArgsBuffer;
        //GPUBufferAllocator* indirectDrawArgsBuffer[3];
        ResourceHandle<ComputeShader> IndirectDrawCombineShaderHandler;
        
    };
```

### File: `Runtime/Renderer/BatchManager.h`
```cpp
namespace EngineCore
{
    struct DrawIndirectParam
    {
        uint32_t indexCount = 0; // 比如这个Mesh有300个索引
        uint32_t startIndexInInstanceDataList = 0; // visiblityBuffer中的index
        uint32_t indexInDrawIndirectList = 0; //indirectDrawCallBuffer中的index
        uint32_t startIndexLocation;    // mesh big buffer 中的index index
        uint32_t baseVertexLocation;    // mesh big buffer 中的vertex index；
        DrawIndirectParam(uint32_t indexCount, uint32_t startIndex, uint32_t baseVertex)
            : indexCount(indexCount), startIndexLocation(startIndex), baseVertexLocation(baseVertex)
        {

        }

        DrawIndirectParam() = default;
    };
```
...
```cpp
        Mesh* mesh;
        DrawIndirectContext() = default;
        DrawIndirectContext(int index, Material* mat, Mesh* mesh) : material(mat), mesh(mesh), batchIndex(index) {}
```
...
```cpp

    class MeshRenderer;
    class BatchManager
    {
    public:
        static BatchManager* GetInstance()
        {
            if (s_Instance == nullptr)
            {
                Create();
            }
            return s_Instance;
        }
        void TryAddBatches(AssetID meshID, AssetID materialID, uint32_t layer);
        void TryDecreaseBatches(AssetID meshID, AssetID materialID, uint32_t layer);
        std::unordered_map<uint64_t, DrawIndirectParam> drawIndirectParamMap;
        std::unordered_map<uint64_t, DrawIndirectContext> drawIndirectContextMap;
        std::unordered_map<uint64_t, int> BatchMap;
        // 记录Batch的顺序，保证每次GetBatchInfo得到的数据稳定， 只会添加
        std::vector<uint64_t> BatchList;

        std::vector<RenderProxy> GetAvaliableRenderProxyList(AssetID meshID, AssetID materialID, uint32_t layer);

        vector<IndirectDrawSource> GetBatchInfo();

        inline const std::unordered_map<uint64_t, DrawIndirectContext>& GetDrawIndirectContextMap() const { return drawIndirectContextMap; }

        inline std::unordered_map<uint64_t, DrawIndirectParam>& GetDrawIndirectParamMap() { return drawIndirectParamMap; }
    private:
        uint64_t GetBatchHash(AssetID meshID, AssetID materialID, uint32_t layer);

        static void Create();
        static BatchManager* s_Instance;

    };
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
        // CPU Prepare Data.
        virtual void Prepare(RenderContext& context) = 0;

        // first record drawInfo then sync data to RenderAPI, finally Submit to execute the
        // Submit To RenderThread
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
    protected:
        void IssueRenderCommandCommon(const RenderPassInfo& passInfo,
                                      const std::vector<RenderBatch>& batches
                                      );
    public:
        string name;
        RenderPassInfo mRenderPassInfo;
    };
} // namespace EngineCore
```

### File: `Runtime/Renderer/RenderPath/LagacyRenderPipeline.h`
```cpp
namespace EngineCore
{
    class LagacyRenderPipeline : public IRenderPipeline
    {
    public:
        virtual ~LagacyRenderPipeline() override {};
        virtual void Prepare(RenderContext& context) override;
        virtual void RecordAndFlush(RenderContext& context) override;
        //virtual void Record(const CommandStream& cmdStream, RenderContext& context) override;
    
    };
```

### File: `Runtime/Renderer/RenderPath/IRenderPipeline.h`
```cpp
namespace EngineCore
{
    class Renderer;
    class IRenderPipeline
    {
    public:
        virtual ~IRenderPipeline() = default;
        virtual void Prepare(RenderContext& context) = 0;
        virtual void RecordAndFlush(RenderContext& context) = 0;
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
        void PrepareFrame(uint32_t frameID, const SceneDelta& delta);
        static void Create();
        void BuildFrame();
        void WaitForFrameAvaliable(uint32_t frameID);
        void EndFrame();
        
        static void OnResize(int width, int height);
        static void OnDrawGUI();
        static void Destory();
        RenderEngine(){};
        ~RenderEngine(){};
        //static void WaitForLastFrameFinished();
        //static void SignalMainThreadSubmited();
        inline CPUScene& GetCPUScene(){return mCPUScene;}
        inline GPUScene& GetGPUScene(){return mGPUScene;}
        inline void SetCurrentFrame(uint32_t currentFrame)
        {
            mCPUScene.SetCurrentFrame(currentFrame);
            mGPUScene.SetCurrentFrame(currentFrame);
        }

        inline FrameTicket* GetCurrentFrameTicket(uint32_t frameID)
        {
            return &mFrameTicket[frameID % 3];
        }

        inline uint32_t GetCurrentFrame(){ return mCurrentFrameID;}
    private:
        IRenderPipeline* mCurrentRenderPipeline;
        
        static std::unique_ptr<RenderEngine> s_Instance;
        RenderContext renderContext;
        UploadPagePool* mUploadPagePool;
        GPUScene mGPUScene;
        CPUScene mCPUScene;
        static constexpr int  MAX_FRAME_INFLIGHT = 3;
        
        FrameTicket* mCurrFrameTicket;
        FrameTicket mFrameTicket[MAX_FRAME_INFLIGHT];
        uint32_t mCurrentFrameID = 0;

        void ComsumeDirtySceneRenderNode(const SceneDelta& delta);
    };
```

### File: `Assets/Shader/GPUCulling.hlsl`
```hlsl
// 2. 常量缓冲：视锥体平面和物体总数
// register(b0) -> 对应 C++ 的 CBV
cbuffer CullingParams : register(b0)
{
    // 视锥体的6个面：Left, Right, Bottom, Top, Near, Far
    // 这里的 float4 存储平面方程: Ax + By + Cz + D = 0
    // xyz 为法线(指向视锥体内侧), w 为距离 D
    float4 g_FrustumPlanes[6]; 
    
    // 需要剔除的实例总数
    uint g_TotalInstanceCount; 
};
```
...
```hlsl


struct PerObjectData
{
    float4x4 objectToWorld;
    uint matIndex;
    uint renderProxyStartIndex;
    uint renderProxyCount;
    uint padding; // 显式填充 12 字节，确保 C++ (72字节) 与 HLSL 布局严格一致
};
```
...
```hlsl
    {
        uint instanceCount;
        InterlockedAdd(g_IndirectDrawCallArgs[batchID].InstanceCount, 1, instanceCount);
        // 执行后：InstanceCount 原子 +1；instanceCount 是加之前的值

        uint indexStart = g_IndirectDrawCallArgs[batchID].StartInstanceLocation;
        uint currentIndex = indexStart + instanceCount;
        // 4. 如果可见，将索引追加到输出列表
        // AppendStructuredBuffer 会自动处理原子计数
        //g_VisibleInstanceIndices.Append(instanceIndex);
        g_VisibleInstanceIndices[currentIndex] = instanceIndex;
    }

}
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

        IGPUBuffer* IndirectDrawArgsBuffer = nullptr;
        IGPUBuffer* IndirectDrawCountCountBuffer = nullptr;
        IGPUBuffer* CullingParamBuffer = nullptr;
        uint32_t currFrameIndex = 0;
    private:

    };
```