# Architecture Digest: MODEL
> Auto-generated. Focus: Runtime/Graphics, Mesh, GeometryManager, MeshFilter, Scene, D3D12RenderAPI, RenderAPI, MetaFactory, MetaLoader, MeshRenderer, BistroSceneLoader

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 模型与几何系统关注Mesh/GeometryManager与GPU上传路径。
- 提取网格数据组织、索引/顶点布局与批处理入口。

## Key Files Index
- `[66]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[64]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[59]` **Runtime/Graphics/GPUSceneManager.cpp** *(Content Included)*
- `[55]` **Runtime/GameObject/MeshRenderer.cpp** *(Content Included)*
- `[55]` **Runtime/Scene/BistroSceneLoader.h** *(Content Included)*
- `[53]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[53]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[52]` **Runtime/GameObject/MeshFilter.h** *(Content Included)*
- `[51]` **Runtime/GameObject/MeshFilter.cpp** *(Content Included)*
- `[44]` **Runtime/Graphics/GeometryManager.cpp** *(Content Included)*
- `[44]` **Runtime/Graphics/Mesh.cpp** *(Content Included)*
- `[42]` **Runtime/Graphics/GeometryManager.h** *(Content Included)*
- `[42]` **Runtime/Graphics/Mesh.h** *(Content Included)*
- `[42]` **Runtime/Scene/Scene.h** *(Content Included)*
- `[42]` **Runtime/Serialization/SceneLoader.h** *(Content Included)*
- `[41]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[40]` **Runtime/Scene/Scene.cpp** *(Content Included)*
- `[37]` **Runtime/Graphics/MeshUtils.cpp** *(Content Included)*
- `[36]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[36]` **Runtime/Graphics/MeshUtils.h** *(Content Included)*
- `[33]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[32]` **Runtime/Scene/SceneManager.h**
- `[29]` **Runtime/Renderer/RenderAPI.h**
- `[27]` **Runtime/Renderer/RenderAPI.cpp**
- `[27]` **Runtime/Serialization/MeshLoader.h**
- `[27]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[24]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[23]` **Runtime/Renderer/RenderEngine.cpp**
- `[22]` **Editor/Panel/EditorMainBar.cpp**
- `[20]` **Runtime/Entry.cpp**
- `[19]` **Runtime/Renderer/Culling.cpp**
- `[19]` **Runtime/Renderer/RenderContext.cpp**
- `[17]` **Runtime/Core/Game.cpp**
- `[17]` **Runtime/Renderer/BatchManager.h**
- `[17]` **Runtime/Renderer/Renderer.h**
- `[15]` **Runtime/Graphics/Material.cpp**
- `[15]` **Runtime/Renderer/BatchManager.cpp**
- `[13]` **Runtime/Core/PublicStruct.h**
- `[13]` **Runtime/Graphics/GPUBufferAllocator.cpp**
- `[12]` **Runtime/Graphics/ComputeShader.cpp**
- `[12]` **Runtime/Graphics/ComputeShader.h**
- `[12]` **Runtime/Graphics/GPUBufferAllocator.h**
- `[12]` **Runtime/Graphics/GPUTexture.h**
- `[12]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[12]` **Runtime/Graphics/IGPUResource.h**
- `[12]` **Runtime/Graphics/Material.h**
- `[12]` **Runtime/Graphics/RenderTexture.cpp**
- `[12]` **Runtime/Graphics/RenderTexture.h**
- `[12]` **Runtime/Graphics/Shader.cpp**
- `[12]` **Runtime/Graphics/Shader.h**
- `[12]` **Runtime/Graphics/Texture.cpp**
- `[12]` **Runtime/Graphics/Texture.h**
- `[11]` **Runtime/GameObject/GameObject.h**
- `[10]` **Runtime/Renderer/Renderer.cpp**
- `[10]` **Runtime/Platforms/D3D12/D3D12DescAllocator.cpp**
- `[10]` **Editor/Panel/EditorMainBar.h**
- `[9]` **Runtime/Renderer/RenderSorter.h**
- `[9]` **Runtime/Resources/ResourceManager.cpp**
- `[9]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[9]` **Editor/D3D12/D3D12EditorGUIManager.cpp**

## Evidence & Implementation Details

### File: `Runtime/Scene/BistroSceneLoader.cpp`
```cpp
            Scene* res = static_cast<Scene*>(sceneLoader.Load(binPath));
            // todo： 这个地方有加载时序问题。。
            SceneManager::GetInstance()->SetCurrentScene(res);

            // 刷新所有Transform的world position，避免父子节点关系建立后的延迟更新问题
            for (auto& gameObject : res->allObjList) 
            {
                if (gameObject != nullptr && gameObject->transform != nullptr)
                {
                    gameObject->transform->UpdateTransform();
                }
                gameObject->transform->isDirty = true;
            }
```
...
```cpp
    }

    void CollectMeshes(GameObject* node, std::vector<Mesh*>& distinctMeshes, std::unordered_map<Mesh*, int>& meshMap) {
        MeshFilter* mf = node->GetComponent<MeshFilter>();
        if (mf && mf->mMeshHandle.IsValid()) {
            Mesh* mesh = mf->mMeshHandle.Get();
            if (meshMap.find(mesh) == meshMap.end()) {
                meshMap[mesh] = (int)distinctMeshes.size();
                distinctMeshes.push_back(mesh);
            }
        }

        for (auto child : node->GetChildren()) {
            CollectMeshes(child, distinctMeshes, meshMap);
        }
    }
```
...
```cpp
        int meshID = -1;
        MeshFilter* mf = node->GetComponent<MeshFilter>();
        if (mf && mf->mMeshHandle.IsValid()) {
            Mesh* mesh = mf->mMeshHandle.Get();
            if (meshMap.count(mesh)) {
                meshID = meshMap[mesh];
            }
        }
```
...
```cpp

        for (auto child : children) {
            SaveNode(child, out, meshMap);
        }
    }

    void BistroSceneLoader::SaveToCache(Scene* scene, const std::string& path) {
```
...
```cpp
        // Find BistroRoot - assumming it's one of the roots or we just take the first one?
        // BistroSceneLoader creates "BistroRoot"
        for (auto go : scene->rootObjList) {
            if (go->name == "BistroRoot") {
                root = go;
                break;
            }
        }
```

### File: `Runtime/Platforms/D3D12/D3D12RenderAPI.cpp`
```cpp
    void D3D12RenderAPI::RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand)
    {
        ASSERT(payloadDrawCommand.mesh != nullptr);
        Mesh* mesh = payloadDrawCommand.mesh;
        auto indexAllocation = mesh->indexAllocation;

        mCommandList->DrawIndexedInstanced(indexAllocation->size / sizeof(uint32_t), payloadDrawCommand.count, 0, 0, 0);
    }

    void D3D12RenderAPI::RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial)
    {
```
...
```cpp
    {
        Mesh* mesh = payloadSetVBIB.mesh;
        ASSERT(mesh != nullptr);
        auto* vertexAllocation = mesh->vertexAllocation;
        auto* indexAllocation = mesh->indexAllocation;

        D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
```
...
```cpp
    void D3D12RenderAPI::RenderAPIDrawInstanceCmd(Payload_DrawInstancedCommand setDrawInstanceCmd)
    {
        ASSERT(setDrawInstanceCmd.mesh != nullptr);
        Mesh* mesh = setDrawInstanceCmd.mesh;
        auto indexAllocation = mesh->indexAllocation;


        mCommandList->SetGraphicsRoot32BitConstants((UINT)RootSigSlot::DrawIndiceConstant, 1, &setDrawInstanceCmd.perDrawOffset, 0);
        mCommandList->DrawIndexedInstanced(indexAllocation->size / sizeof(uint32_t), setDrawInstanceCmd.count, 0, 0, setDrawInstanceCmd.perDrawOffset);
        //ASSERT_MSG(false, "Not Implemented!");
    }

    void D3D12RenderAPI::RenderAPISetPerPassData(Payload_SetPerPassData setPerPassData)
    {
```

### File: `Runtime/Graphics/GPUSceneManager.cpp`
```cpp
    {
        if(sInstance != nullptr) return sInstance;
        GPUSceneManager::Create();
        return sInstance;   
    }

    void GPUSceneManager::Create()
    {
```
...
```cpp

        vector<DrawIndirectArgs> drawIndirectArgsList = BatchManager::GetInstance()->GetBatchInfo();
        UpdateRenderProxyBuffer(renderSceneData.materialDirtyList);
        UpdateAABBandPerObjectBuffer(renderSceneData.transformDirtyList, renderSceneData.materialDirtyList);

        // 重置visibilityBuffer
        vector<uint8_t> empty;
        empty.resize(4 * 10000, 0);
        visibilityBuffer->UploadBuffer(visiblityAlloc, empty.data(), empty.size());
        visibilityBuffer->Reset();
    }

    BufferAllocation GPUSceneManager::GetSinglePerMaterialData()
    {
```

### File: `Runtime/Scene/BistroSceneLoader.h`
```cpp
    class Node;
    class Model;
    class Mesh;
}

namespace EngineCore {
```
...
```cpp
    class Scene;

    class BistroSceneLoader {
    public:
        static Scene* Load(const std::string& path);
        static void SaveToCache(Scene* scene, const std::string& path);
        static Scene* LoadFromCache(const std::string& path);
        static ResourceHandle<Material> commonMatHandle;
        
    private:
        Scene* LoadInternal(const std::string& path);
        void ProcessNode(const tinygltf::Node& node, const tinygltf::Model& model, GameObject* parent, Scene* targetScene);
        void ProcessMesh(int meshIndex, const tinygltf::Model& model, GameObject* go, Scene* targetScene);
        void ProcessMaterials(const tinygltf::Model& model);
        void ProcessTexture(const tinygltf::Model& model);
        void AttachMaterialToGameObject(GameObject* gameObject, int materialIndex);
        AssetID GetTextureAssetID(const tinygltf::Model& model, int textureIndex);
        std::map<int, std::vector<std::pair<ResourceHandle<Mesh>, int>>> m_MeshCache;
        std::vector<AssetID> m_ImageIndexToID;
        std::vector<ResourceHandle<Material>> m_MaterialMap;
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

### File: `Runtime/Platforms/D3D12/D3D12RenderAPI.h`
```cpp
namespace EngineCore
{
    class D3D12RenderAPI : public RenderAPI
    {
    public:

        D3D12RenderAPI();
        ~D3D12RenderAPI(){};

        virtual void CompileShader(const string& path, Shader* shader) override;
        virtual void CompileComputeShader(const string& path, ComputeShader* csShader) override;

        inline TD3D12Fence* GetFrameFence() { return mFrameFence; };
        virtual IGPUTexture* CreateTextureBuffer(unsigned char* data, const TextureDesc& textureDesc) override;
        virtual IGPUTexture* CreateRenderTexture(const TextureDesc& textureDesc) override;
        
        // 渲染线程调用接口
        virtual void RenderAPIBeginFrame() override;
        virtual void RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT) override;
        virtual void RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand) override;
        virtual void RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial) override;
        virtual void RenderAPISetBindlessMat(Payload_SetBindlessMat payloadSetBindlessMat) override;
        virtual void RenderAPISetBindLessMeshIB() override;
        virtual void RenderAPISetRenderState(Payload_SetRenderState payloadSetRenderState) override;
        virtual void RenderAPISetSissorRect(Payload_SetSissorRect payloadSetSissorrect) override;
        virtual void RenderAPISetVBIB(Payload_SetVBIB payloadSetVBIB) override;
        virtual void RenderAPISetViewPort(Payload_SetViewPort payloadSetViewport) override;
        virtual void RenderAPIWindowResize(Payload_WindowResize payloadWindowResize) override;
        virtual void RenderAPISubmit() override;
        virtual void RenderAPIPresentFrame() override;
        virtual void RenderAPISetPerDrawData(Payload_SetPerDrawData setPerDrawData) override;
        virtual void RenderAPIDrawInstanceCmd(Payload_DrawInstancedCommand setDrawInstanceCmd) override;
        virtual void RenderAPISetPerPassData(Payload_SetPerPassData setPerPassData) override;
        virtual void RenderAPISetPerFrameData(Payload_SetPerFrameData setPerFrameData) override;
        virtual void RenderAPICopyRegion(Payload_CopyBufferRegion copyBufferRegion) override;
        virtual void RenderAPIDispatchComputeShader(Payload_DispatchComputeShader dispatchComputeShader) override;
        virtual void RenderAPISetBufferResourceState(Payload_SetBufferResourceState bufferResourceState) override;
        virtual void RenderAPIExecuteIndirect(Payload_DrawIndirect drawIndirect) override;

        
        virtual void CreateGlobalConstantBuffer(uint32_t enumID, uint32_t size) override;
        virtual RenderTexture* GetCurrentBackBuffer() override;

        virtual void SetGlobalDataImpl(uint32_t bufferID, uint32_t offset, uint32_t size, const void* value) override;

        TD3D12ConstantBuffer CreateConstantBuffer(uint32_t size);

        Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
        UINT mRtvDescriptorSize = 0;
        UINT mDsvDescriptorSize = 0;
        UINT mCbvSrvUavDescriptorSize = 0;
        const int MAX_FRAME_INFLIAGHT = 3;
        DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT mCurrBackBuffer = 0;
        static const int SwapChainBufferCount = 3;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
        // 给底层使用的具体资源
        D3D12Texture mBackBuffer[SwapChainBufferCount];
        // 一个壳，上层用，IGPUTexture = mBackBuffer
        RenderTexture mBackBufferProxyRenderTexture;
        D3D12Texture mBackBufferProxy;
        
        TD3D12Fence* mFrameFence;
        TD3D12Fence* mImediatelyFence;

        D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderAPI::CurrentBackBufferView()const
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(
                mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
                mCurrBackBuffer,
                mRtvDescriptorSize);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderAPI::DepthStencilView()const
        {
            return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
        }

        ID3D12Resource* D3D12RenderAPI::CurrentBackBuffer()const
        {
```

### File: `Runtime/GameObject/MeshFilter.h`
```cpp
namespace EngineCore
{
    class MeshFilter : public Component
    {
        class GameObejct;
    public:
        MeshFilter() = default;
        MeshFilter(GameObject* gamObject);

        virtual ~MeshFilter() override;
        static ComponentType GetStaticType() { return ComponentType::MeshFilter; };
        virtual ComponentType GetType() const override{ return ComponentType::MeshFilter; };
        void OnLoadResourceFinished();
    public:
        ResourceHandle<Mesh> mMeshHandle;
        
        virtual const char* GetScriptName() const override { return "MeshFilter"; }

        uint32_t GetHash()
        {
            return mMeshHandle->GetInstanceID();
        }
    private:
        uint32_t hash;
    };
```

### File: `Runtime/Graphics/GeometryManager.h`
```cpp
namespace EngineCore
{
    class GeometryManager
    {
    public:
        GeometryManager();
        ~GeometryManager()
        {
            delete m_GlobalVertexBufferAllocator;
            delete m_GLobalIndexBufferAllocator;
        }
        static GeometryManager* GetInstance()
        {
            if(s_Instance == nullptr)
            {
                s_Instance = new GeometryManager();
            }
            return s_Instance;
        }
        
        MeshBufferAllocation* AllocateVertexBuffer(void* data, int size);
        MeshBufferAllocation* AllocateIndexBuffer(void* data, int size);

        void FreeVertexAllocation(MeshBufferAllocation* allocation);
        void FreeIndexAllocation(MeshBufferAllocation* allocation);
        inline IGPUBuffer* GetVertexBuffer(){ return m_GlobalVertexBufferAllocator->GetGPUBuffer();}
        inline IGPUBuffer* GetIndexBuffer(){ return m_GLobalIndexBufferAllocator->GetGPUBuffer();}
        inline uint32_t GetIndexBufferSize(){ return m_GLobalIndexBufferAllocator->bufferDesc.size;}    
    private:
        GPUBufferAllocator* m_GlobalVertexBufferAllocator;
        GPUBufferAllocator* m_GLobalIndexBufferAllocator;
        static GeometryManager* s_Instance;
    };
```

### File: `Runtime/Graphics/Mesh.h`
```cpp
    };

    struct MeshBufferAllocation
    {
        IGPUBuffer* buffer = nullptr;
        // 当前数据开始位置， 可以直接绑定
        uint64_t gpuAddress = 0;
        uint64_t offset =0;
        uint64_t size = 0;
        uint32_t stride = 0;
        bool isValid = false;
        struct MeshBufferAllocation() = default;
        struct MeshBufferAllocation(IGPUBuffer* buffer, uint64_t gpuAddress, uint64_t offset, uint64_t size, uint64_t stride)
            :buffer(buffer), gpuAddress(gpuAddress), offset(offset), size(size), stride(stride)
        {
            isValid = true;
        }
    };
```
...
```cpp

        Mesh() = default;
        Mesh(Primitive primitiveType);
        MeshBufferAllocation* vertexAllocation;
        MeshBufferAllocation* indexAllocation;
        void UploadMeshToGPU();

        AABB bounds;
        std::vector<Vertex> vertex;
        std::vector<int> index;
        std::vector<InputLayout> layout;
        bool isDynamic = true;
        virtual void OnLoadComplete() override { UploadMeshToGPU(); };
```
...
```cpp
        void ProcessNode(aiNode* node, const aiScene* scene);
        void LoadAiMesh(const string& path);
        void ProcessMesh(aiMesh* aiMesh, const aiScene* scene);

    };

}
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
            {
                // 直接访问安全，因为meshrenderer为空的不会加到这里。
                auto* renderer = meshRendererList[index];
                aabbList[index] = renderer->worldBounds;
                objectToWorldMatrixList[index] = renderer->gameObject->transform->GetWorldMatrix();
            }
            // todo: 暂时没有这个逻辑， 比如material切换pso，切换vao这种
            //materialDirtyList...
        }
    };

    class Scene : public Resource
    {
    public:
        Scene();
        ~Scene();
        Scene(const std::string& name):name(name){};
        void Open();
        void Close(){};
        void Update();
```
...
```cpp
        GameObject* FindGameObject(const std::string& name);
        GameObject* CreateGameObject(const std::string& name);
        void Scene::DestroyGameObject(const std::string& name);

        void AddCamToStack(Camera* cam);
        inline void SetMainCamera(Camera* cam) { mainCamera = cam; }
```
...
```cpp
        
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
} // namespace EngineCore
```

### File: `Runtime/Serialization/SceneLoader.h`
```cpp
namespace EngineCore
{
    struct SceneSerializedNode
    {
        char name[64];         // 固定长度名字
        int32_t parentIndex = -1;
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;

        uint64_t meshID = 0;
        uint64_t materialID = 0;

    };
```
...
```cpp
        void SaveSceneToBin(const Scene* scene, const std::string& relativePath, uint32_t id)
        {
            ASSERT(scene && scene->allObjList.size() > 0);
            std::string binPath = PathSettings::ResolveAssetPath(relativePath);
            std::ofstream out(binPath, std::ios::binary);

            AssetHeader header;
            header.assetID =id;
            header.type = 0;
            StreamHelper::Write(out, header);

            std::vector<SceneSerializedNode> linearNode;
            std::unordered_map<GameObject*, uint32_t> gameObjectMap;
            for(int i = 0; i < scene->rootObjList.size(); i++)
            {
                GameObject* gameObject = scene->rootObjList[i];
                
                SerilizedNode(gameObject, gameObjectMap, linearNode);
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

### File: `Runtime/Graphics/MeshUtils.h`
```cpp
namespace EngineCore
{
    class MeshUtils
    {
    public:
        static void GetFullScreenQuad(Mesh* modelData);
    private:

    };
```