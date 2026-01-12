# Architecture Digest: GameLogicUpdate
> Auto-generated. Focus: Game, SceneManager, Scene, RenderEngine, LagacyRenderPath, GPUSceneRenderPath, Renderer, RenderContext, FinalBlitPass, GPUSceneRenderPass, OpaqueRenderPass

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Key Files Index
- `[73]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h** *(Content Included)*
- `[62]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h** *(Content Included)*
- `[58]` **Runtime/Scene/SceneManager.h** *(Content Included)*
- `[52]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[51]` **Runtime/Renderer/RenderEngine.h** *(Content Included)*
- `[51]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h** *(Content Included)*
- `[41]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[40]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h** *(Content Included)*
- `[39]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h** *(Content Included)*
- `[38]` **Runtime/Renderer/RenderContext.h** *(Content Included)*
- `[37]` **Runtime/Scene/Scene.h** *(Content Included)*
- `[33]` **Runtime/GameObject/GameObject.h**
- `[33]` **Runtime/Scene/BistroSceneLoader.h**
- `[30]` **Runtime/Renderer/Renderer.h**
- `[27]` **Runtime/Core/Game.h**
- `[24]` **Editor/Panel/EditorGameViewPanel.h**
- `[20]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[19]` **Runtime/GameObject/Transform.h**
- `[19]` **Runtime/Renderer/BatchManager.h**
- `[17]` **Runtime/Renderer/RenderSorter.h**
- `[16]` **Runtime/Renderer/RenderPath/IRenderPath.h**
- `[15]` **Runtime/GameObject/Component.h**
- `[15]` **Runtime/Renderer/Culling.h**
- `[14]` **Runtime/GameObject/MeshFilter.h**
- `[14]` **Runtime/GameObject/MonoBehaviour.h**
- `[14]` **Runtime/Renderer/RenderAPI.h**
- `[14]` **Runtime/Serialization/MetaFactory.h**
- `[13]` **Runtime/GameObject/Camera.h**
- `[13]` **Runtime/GameObject/ComponentType.h**
- `[13]` **Runtime/Renderer/RenderCommand.h**
- `[12]` **Runtime/Renderer/PerDrawAllocator.h**
- `[12]` **Runtime/Renderer/RenderStruct.h**
- `[12]` **Runtime/Renderer/RenderUniforms.h**
- `[12]` **Runtime/Renderer/SPSCRingBuffer.h**
- `[11]` **Editor/Panel/EditorMainBar.h**
- `[7]` **Editor/EditorGUIManager.h**
- `[7]` **Editor/EditorSettings.h**
- `[7]` **Runtime/EngineCore.h**
- `[7]` **Runtime/Core/PublicStruct.h**
- `[7]` **Runtime/Graphics/Mesh.h**
- `[7]` **Editor/Panel/EditorHierarchyPanel.h**
- `[6]` **Runtime/Serialization/ComponentFactory.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12RootSignature.h**
- `[4]` **Runtime/Scripts/CameraController.h**
- `[3]` **Runtime/Graphics/GPUBufferAllocator.h**
- `[3]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[3]` **Runtime/Graphics/Material.h**
- `[3]` **Runtime/Graphics/MeshUtils.h**
- `[3]` **Runtime/Platforms/D3D12/D3D12PSO.h**
- `[3]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[3]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[3]` **Editor/Panel/EditorInspectorPanel.h**
- `[2]` **Runtime/CoreAssert.h**
- `[2]` **Runtime/PreCompiledHeader.h**
- `[2]` **Runtime/Core/InstanceID.h**
- `[2]` **Runtime/Core/Object.h**
- `[2]` **Runtime/Core/Profiler.h**
- `[2]` **Runtime/Core/PublicEnum.h**
- `[2]` **Runtime/Graphics/ComputeShader.h**
- `[2]` **Runtime/Graphics/GeometryManager.h**

## Evidence & Implementation Details

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
            indirectDrawArgsBuffer->UploadBuffer(indirectDrawArgsAlloc, batchInfo.data(), batchInfo.size() * sizeof(DrawIndirectArgs));
```
...
```cpp
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
        virtual json SerializedFields() const override {
            return json{
                {"MatHandle", mShardMatHandler},
            };
        }
        
        virtual void DeserializedFields(const json& data) override;
        
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
        AABB worldBounds;
        uint32_t sceneRenderNodeIndex = UINT32_MAX;
        bool materialDirty = true;
		
        void TryAddtoBatchManager();
        uint32_t renderLayer = 1;
    private:
        ResourceHandle<Material> mShardMatHandler;
        ResourceHandle<Material> mInstanceMatHandler;

    };

}
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

### File: `Runtime/Scene/Scene.h`
```cpp
namespace EngineCore
{
    struct RenderSceneData
    {
        vector<MeshRenderer*> meshRendererList;
        vector<MeshFilter*> meshFilterList;
        vector<AABB> aabbList;
        vector<Matrix4x4> objectToWorldMatrixList;
        vector<uint32_t> layerList;

        // 每帧重置
        vector<uint32_t> transformDirtyList;
        vector<uint32_t> materialDirtyList;
        struct RenderSceneData() = default;
        inline void SyncData(MeshRenderer* renderer, uint32_t index)
        {
            meshRendererList[index] = renderer;
            if (renderer && renderer->gameObject)
            {
                aabbList[index] = renderer->worldBounds;
                objectToWorldMatrixList[index] = renderer->gameObject->transform->GetWorldMatrix();
                auto* meshFilter = renderer->gameObject->GetComponent<MeshFilter>();
                if(meshFilter != nullptr)
                {
                    meshFilterList[index] = meshFilter;
                }
                else
                {
                    meshFilterList[index] = nullptr;
                }
            }
        }

        inline void PushNewData() 
        {
            meshRendererList.emplace_back();
            meshFilterList.emplace_back();
            aabbList.emplace_back();
            objectToWorldMatrixList.emplace_back();
            layerList.emplace_back();
        }


        inline void DeleteData(uint32_t index)
        {
            meshRendererList[index] = nullptr;
            meshFilterList[index] = nullptr;

            // 后续删除RenderProxy
        }

        inline void ClearDirtyList()
        {
            materialDirtyList.clear();
            transformDirtyList.clear();
        }

        inline void UpdateDirtyRenderNode()
        {
            for(uint32_t index : transformDirtyList)
```
...
```cpp
    };

    class Scene : Object
    {
    public:
        Scene();
        ~Scene();
        Scene(const std::string& name):name(name){};
        void Open();
        void Close(){};
        void Update();
        void EndFrame();
        GameObject* FindGameObject(const std::string& name);
        GameObject* CreateGameObject(const std::string& name);
        void Scene::DestroyGameObject(const std::string& name);

        void AddCamToStack(Camera* cam);
        inline void SetMainCamera(Camera* cam) { mainCamera = cam; }
        inline Camera* GetMainCamera() { return mainCamera; }
        
        // 只在GameObject层用到，析构和SetParent的时候， 创建的时候调用
        void RemoveGameObjectToSceneList(GameObject* object);
        void AddGameObjectToSceneList(GameObject* object);

        void AddRootGameObject(GameObject* object);
        void TryRemoveRootGameObject(GameObject* object);
        
        //todo: 先用vector写死，后面要用priorityqueue之类的
        std::vector<Camera*> cameraStack;

        void RunLogicUpdate();
        void RunTransformUpdate();
        void RunRecordDirtyRenderNode();


        // todo:
        // 材质更新的时候， 也需要去调用这个逻辑，比如changeRenderNodeInfo之类的，
        int AddNewRenderNodeToCurrentScene(MeshRenderer* renderer);
        void DeleteRenderNodeFromCurrentScene(uint32_t index);
        
    public:
        int m_CurrentSceneRenderNodeIndex = 0;
        std::queue<uint32_t> m_FreeSceneNode;
        std::string name;
        std::vector<GameObject*> allObjList;
        std::vector<GameObject*> rootObjList;
        bool enabled = true;
        Camera* mainCamera = nullptr;
        RenderSceneData renderSceneData;
    private:
    };    
```