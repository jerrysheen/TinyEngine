# Architecture Digest: GameLogicUpdate
> Auto-generated. Focus: Runtime/Core/Game, Game, SceneManager, Scene, RenderEngine, LagacyRenderPath, GPUSceneRenderPath, Renderer, RenderContext, FinalBlitPass, GPUSceneRenderPass, OpaqueRenderPass, RenderPipeLine

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 围绕主循环的Update/Render/EndFrame与场景刷新顺序。
- 关注渲染路径选择与关键Pass调用链。

## Key Files Index
- `[85]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp** *(Content Included)*
- `[75]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[73]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h** *(Content Included)*
- `[72]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h** *(Content Included)*
- `[64]` **Runtime/Graphics/GPUSceneManager.cpp** *(Content Included)*
- `[64]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[63]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp** *(Content Included)*
- `[61]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[58]` **Runtime/Scene/SceneManager.h** *(Content Included)*
- `[55]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp** *(Content Included)*
- `[53]` **Runtime/Renderer/RenderContext.cpp** *(Content Included)*
- `[52]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[51]` **Runtime/Renderer/RenderEngine.h** *(Content Included)*
- `[51]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h** *(Content Included)*
- `[50]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h** *(Content Included)*
- `[49]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h** *(Content Included)*
- `[47]` **Runtime/GameObject/MeshRenderer.cpp** *(Content Included)*
- `[41]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[38]` **Runtime/Renderer/RenderContext.h** *(Content Included)*
- `[38]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[37]` **Runtime/Core/Game.h**
- `[37]` **Runtime/Scene/Scene.h**
- `[37]` **Runtime/Serialization/SceneLoader.h**
- `[35]` **Runtime/Renderer/Renderer.h**
- `[35]` **Runtime/Scene/Scene.cpp**
- `[33]` **Runtime/GameObject/GameObject.h**
- `[33]` **Runtime/Scene/BistroSceneLoader.h**
- `[32]` **Runtime/GameObject/GameObject.cpp**
- `[30]` **Runtime/GameObject/Camera.cpp**
- `[30]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[28]` **Runtime/Renderer/Renderer.cpp**
- `[28]` **Editor/Panel/EditorGameViewPanel.cpp**
- `[27]` **Runtime/Renderer/Culling.cpp**
- `[24]` **Runtime/Entry.cpp**
- `[24]` **Editor/Panel/EditorGameViewPanel.h**
- `[20]` **Runtime/Renderer/RenderPipeLine/RenderPass.cpp**
- `[19]` **Runtime/GameObject/Transform.h**
- `[19]` **Runtime/Renderer/BatchManager.cpp**
- `[19]` **Runtime/Renderer/BatchManager.h**
- `[18]` **Editor/Panel/EditorMainBar.cpp**
- `[17]` **Runtime/Renderer/RenderSorter.h**
- `[16]` **Runtime/GameObject/MeshFilter.cpp**
- `[16]` **Runtime/Renderer/RenderPath/IRenderPath.h**
- `[15]` **Runtime/GameObject/Component.h**
- `[15]` **Runtime/Renderer/Culling.h**
- `[15]` **Runtime/Renderer/RenderAPI.h**
- `[14]` **Runtime/GameObject/MeshFilter.h**
- `[14]` **Runtime/GameObject/MonoBehaviour.h**
- `[14]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[13]` **Runtime/GameObject/Camera.h**
- `[13]` **Runtime/GameObject/ComponentType.h**
- `[13]` **Runtime/GameObject/MonoBehaviour.cpp**
- `[13]` **Runtime/GameObject/Transform.cpp**
- `[13]` **Runtime/Renderer/RenderCommand.h**
- `[12]` **Runtime/Renderer/PerDrawAllocator.h**
- `[12]` **Runtime/Renderer/RenderStruct.h**
- `[12]` **Runtime/Renderer/RenderUniforms.h**
- `[12]` **Runtime/Renderer/SPSCRingBuffer.h**
- `[12]` **Editor/Panel/EditorHierarchyPanel.cpp**
- `[11]` **Runtime/GameObject/Component.cpp**

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

### File: `Runtime/Renderer/RenderEngine.cpp`
```cpp
        WindowManager::Create();
        RenderAPI::Create();
        Renderer::Create();
        GPUSceneManager::Create();
    }

    void RenderEngine::Update()
    {
```
...
```cpp
    void RenderEngine::OnResize(int width, int height)
    {
        Renderer::GetInstance()->ResizeWindow(width, height);
    }

    void RenderEngine::Tick()
    {
```
...
```cpp
    {
        // Renderer 的析构函数会自动停止渲染线程
        Renderer::Destroy();
        //RenderAPI::Destroy();
        WindowManager::Destroy();
        GPUSceneManager::GetInstance()->Destroy();
        // 最后销毁 RenderEngine 自己
        if (s_Instance)
        {
            // 先调用析构（智能指针自动管理）
            s_Instance.reset();
            // 此时：
            // 1. 析构函数已完成
            // 2. 内存已释放
            // 3. s_Instance 已经是 nullptr
            // 4. 后续 GetInstance() 不会返回野指针
        }
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

### File: `Runtime/Scene/SceneManager.h`
```cpp
namespace EngineCore
{
    class Scene;
    class SceneManager
    {
        // 允许Manager类访问SceneManager私有函数。
    public:
        void LoadScene();
        void UnloadScene();
        GameObject* CreateGameObject(const std::string& name);
        GameObject* FindGameObject(const std::string& name);

        void RemoveScene(const std::string& name);
        static void Update();
        static void Create();
        static void Destroy();
        static void EndFrame();
        void Init();
    public:
        inline static SceneManager* GetInstance() 
        {
            if (!s_Instance) 
            {
                s_Instance = new SceneManager();
            }
            return s_Instance;
        }

        SceneManager();
        ~SceneManager();

        // todo： 这部分数据也要找地方存， maybe一个Global的渲染处

        Material* blitMaterial;
        Mesh* quadMesh;
        ResourceHandle<Shader> blitShader;
        ResourceHandle<Texture> testTexture;

        inline Scene* GetCurrentScene() 
        { 
            return mCurrentScene; 
        };
        inline void SetCurrentScene(Scene* scene) 
        { 
            mCurrentScene = scene; 
        };
        Scene* AddNewScene(const std::string& name);
        void SwitchSceneTo(const std::string& name);

    private:
        static SceneManager* s_Instance;
        Scene* mCurrentScene = nullptr;
        unordered_map<std::string, Scene*> mSceneMap;
        vector<ResourceHandle<Texture>> texHandler;
    };

}
```

### File: `Runtime/Graphics/GPUSceneManager.h`
```cpp
{

    class GPUSceneManager
    {
    public:
        static GPUSceneManager* GetInstance();
        GPUSceneManager();
        static void Create();
        void Tick();
        void Destroy();
        
        BufferAllocation GetSinglePerMaterialData();
        void RemoveSinglePerMaterialData(const BufferAllocation& bufferalloc);
        void UpdateSinglePerMaterialData(const BufferAllocation& bufferalloc, void* data);

        void TryFreeRenderProxyBlock(uint32_t index);
        void TryCreateRenderProxyBlock(uint32_t index);
        BufferAllocation LagacyRenderPathUploadBatch(void *data, uint32_t size);
        void FlushBatchUploads();
        void UpdateRenderProxyBuffer(const vector<uint32_t>& materialDirtyList);
        void UpdateAABBandPerObjectBuffer(const vector<uint32_t>& transformDirtyList, const vector<uint32_t>& materialDirtyList);

        vector<PerObjectData> perObjectDataBuffer;

        LinearAllocator* perFramelinearMemoryAllocator;

        GPUBufferAllocator* allMaterialDataBuffer;
        GPUBufferAllocator* allObjectDataBuffer;
        GPUBufferAllocator* perFrameBatchBuffer;
        GPUBufferAllocator* allAABBBuffer;
        GPUBufferAllocator* renderProxyBuffer;


        BufferAllocation visiblityAlloc;
        GPUBufferAllocator* visibilityBuffer;
        
        ResourceHandle<ComputeShader> GPUCullingShaderHandler;
    private:
        static GPUSceneManager* sInstance; 
        vector<CopyOp> mPendingBatchCopies;
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

### File: `Runtime/GameObject/MeshRenderer.h`
```cpp
namespace EngineCore
{
    class MeshRenderer : public Component
    {
        class GameObejct;
    public:
        MeshRenderer() = default;
        MeshRenderer(GameObject* gamObject);
        virtual ~MeshRenderer() override;
        static ComponentType GetStaticType() { return ComponentType::MeshRenderer; };
        virtual ComponentType GetType() const override{ return ComponentType::MeshRenderer; };

        virtual const char* GetScriptName() const override { return "MeshRenderer"; }
        
        void SetUpMaterialPropertyBlock();

        inline Material* GetSharedMaterial()
        { 
            return mShardMatHandler.IsValid() ? mShardMatHandler.Get() : nullptr;
        };

        inline void SetSharedMaterial(const ResourceHandle<Material>& mat) 
        {
            mShardMatHandler = mat;
            SetUpMaterialPropertyBlock();
        }

        // return a new Material Instance;
        Material* GetOrCreateMatInstance();
        // 
        ResourceHandle<Material> GetMaterial();
        inline bool HasMaterialOverride() { return mInstanceMatHandler.IsValid(); }

        void UpdateBounds(const AABB& localBounds, const Matrix4x4& worldMatrix);
        uint32_t lastSyncTransformVersion = 0;
        bool shouldUpdateMeshRenderer = true;

        AABB worldBounds;
        uint32_t sceneRenderNodeIndex = UINT32_MAX;
        bool materialDirty = true;
		
        void TryAddtoBatchManager();

        uint32_t renderLayer = 1;
    private:
        ResourceHandle<Material> mShardMatHandler;
        ResourceHandle<Material> mInstanceMatHandler;

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