# Architecture Digest: COMPONENT_SYSTEM
> Auto-generated. Focus: Camera, Component, ComponentType, MeshFilter, MeshRenderer, MonoBehaviour, Transform, Scene

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Key Files Index
- `[54]` **Runtime/GameObject/ComponentType.h** *(Content Included)*
- `[50]` **Runtime/Scene/Scene.cpp** *(Content Included)*
- `[49]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[46]` **Runtime/Serialization/SceneLoader.h** *(Content Included)*
- `[45]` **Runtime/Scene/Scene.h** *(Content Included)*
- `[41]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[38]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[38]` **Runtime/GameObject/Transform.h** *(Content Included)*
- `[37]` **Runtime/GameObject/MeshFilter.h** *(Content Included)*
- `[36]` **Runtime/GameObject/Camera.h** *(Content Included)*
- `[35]` **Runtime/GameObject/Camera.cpp** *(Content Included)*
- `[35]` **Runtime/GameObject/MonoBehaviour.h** *(Content Included)*
- `[32]` **Runtime/Scene/CPUScene.cpp** *(Content Included)*
- `[31]` **Runtime/GameObject/MeshFilter.cpp** *(Content Included)*
- `[31]` **Runtime/GameObject/MeshRenderer.cpp** *(Content Included)*
- `[31]` **Runtime/Scene/SceneStruct.h** *(Content Included)*
- `[30]` **Runtime/GameObject/Transform.cpp** *(Content Included)*
- `[30]` **Runtime/Scene/CPUScene.h** *(Content Included)*
- `[30]` **Runtime/Scene/SceneManager.h** *(Content Included)*
- `[29]` **Runtime/GameObject/Component.h** *(Content Included)*
- `[29]` **Runtime/Scene/BistroSceneLoader.h**
- `[28]` **Runtime/GameObject/GameObject.h**
- `[28]` **Runtime/Scripts/CameraController.h**
- `[28]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.cpp**
- `[27]` **Runtime/Scene/GPUScene.h**
- `[27]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[27]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[25]` **Runtime/Scene/GPUScene.cpp**
- `[25]` **Runtime/Scripts/CameraController.cpp**
- `[24]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[23]` **Runtime/GameObject/Component.cpp**
- `[23]` **Runtime/GameObject/MonoBehaviour.cpp**
- `[23]` **Editor/Panel/EditorMainBar.cpp**
- `[20]` **Runtime/Entry.cpp**
- `[18]` **Runtime/GameObject/GameObject.cpp**
- `[17]` **Runtime/Core/Game.cpp**
- `[14]` **Runtime/Renderer/RenderEngine.cpp**
- `[12]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[11]` **Runtime/Renderer/Culling.cpp**
- `[9]` **Editor/Panel/EditorHierarchyPanel.cpp**
- `[9]` **Editor/Panel/EditorInspectorPanel.cpp**
- `[8]` **Runtime/Graphics/Mesh.h**
- `[8]` **Runtime/Renderer/RenderPath/LagacyRenderPath.cpp**
- `[8]` **Editor/Panel/EditorInspectorPanel.h**
- `[7]` **Runtime/Renderer/FrameContext.cpp**
- `[7]` **Runtime/Renderer/FrameContext.h**
- `[7]` **Runtime/Renderer/RenderEngine.h**
- `[7]` **Runtime/Renderer/Renderer.h**
- `[7]` **Runtime/Renderer/RenderSorter.h**
- `[7]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[7]` **Editor/Panel/EditorMainBar.h**
- `[6]` **Runtime/Core/PublicStruct.h**
- `[6]` **Runtime/Renderer/RenderContext.cpp**
- `[6]` **Runtime/Renderer/RenderContext.h**
- `[6]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[6]` **Assets/Shader/StandardPBR.hlsl**
- `[5]` **Runtime/Graphics/Material.cpp**
- `[5]` **Runtime/Graphics/Mesh.cpp**

## Evidence & Implementation Details

### File: `Runtime/GameObject/ComponentType.h`
```cpp
namespace EngineCore
{
    enum class ComponentType
    {
        MeshRenderer,
        MeshFilter,
        Camera,
        Transform,
        Script
    };
```

### File: `Runtime/Scene/Scene.cpp`
```cpp
        PushLastFrameFreeIndex();
        RunLogicUpdate();
        RunTransformUpdate();
        RunRemoveInvalidDirtyRenderNode();
    }

    void Scene::EndFrame()
    {
```
...
```cpp
    void Scene::PushNewTransformDirtyRoot(Transform *transform)
    {
        ASSERT(transform);
        dirtyRootDepthBucket[transform->GetNodeDepth()].push_back(transform);
    }

    void Scene::RunLogicUpdate()
    {
```
...
```cpp
        uint32_t renderID = renderer->GetCPUWorldIndex();

        NodeDirtyPayload payload(transform);
        ApplyQueueNodeChange(renderID, (uint32_t)NodeDirtyFlags::TransformDirty, payload);        
    }

    void Scene::MarkNodeMeshFilterDirty(MeshFilter *meshFilter)
    {
```

### File: `Runtime/Scene/BistroSceneLoader.cpp`
```cpp
            Scene* res = static_cast<Scene*>(sceneLoader.Load(binPath).resource);
            // todo： 这个地方有加载时序问题。。
            SceneManager::GetInstance()->SetCurrentScene(res);
            // 刷新所有Transform的world position，避免父子节点关系建立后的延迟更新问题
            for (auto& gameObject : res->rootObjList) 
            {
                if (gameObject != nullptr && gameObject->transform != nullptr)
                {
                    gameObject->transform->UpdateRecursively(0);
                }
                gameObject->transform->isDirty = true;
            }
```
...
```cpp
        
        // Transform
        WriteVector3(out, node->transform->GetLocalPosition());
        WriteQuaternion(out, node->transform->GetLocalQuaternion());
        WriteVector3(out, node->transform->GetLocalScale());

        // Mesh
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
    }

    void BistroSceneLoader::SaveToCache(Scene* scene, const std::string& path) {
        std::ofstream out(path, std::ios::binary);
        if (!out) {
            std::cout << "Failed to open cache file for writing: " << path << std::endl;
            return;
        }

        // Header
        const char* magic = "TINY";
        out.write(magic, 4);
        int version = 2;
        out.write(reinterpret_cast<const char*>(&version), sizeof(version));

        GameObject* root = nullptr;
        // Find BistroRoot - assumming it's one of the roots or we just take the first one?
        // BistroSceneLoader creates "BistroRoot"
        for (auto go : scene->rootObjList) {
            if (go->name == "BistroRoot") {
                root = go;
                break;
            }
        }
        
        if (!root && !scene->rootObjList.empty()) {
             root = scene->rootObjList[0];
        }

        if (!root) {
            std::cout << "No root object found to save." << std::endl;
            return;
        }

        // 1. Collect Meshes
        std::vector<Mesh*> distinctMeshes;
        std::unordered_map<Mesh*, int> meshMap;
        CollectMeshes(root, distinctMeshes, meshMap);

        // 2. Save Meshes
        size_t meshCount = distinctMeshes.size();
        out.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));

        for (Mesh* mesh : distinctMeshes) {
            // Bounds
            out.write(reinterpret_cast<const char*>(&mesh->bounds), sizeof(AABB));
            
            // Vertices
            size_t vCount = mesh->vertex.size();
            out.write(reinterpret_cast<const char*>(&vCount), sizeof(vCount));
            if (vCount > 0) {
                out.write(reinterpret_cast<const char*>(mesh->vertex.data()), vCount * sizeof(Vertex));
            }

            // Indices
            size_t iCount = mesh->index.size();
            out.write(reinterpret_cast<const char*>(&iCount), sizeof(iCount));
            if (iCount > 0) {
                out.write(reinterpret_cast<const char*>(mesh->index.data()), iCount * sizeof(int));
            }

            // Layout
            size_t lCount = mesh->layout.size();
            out.write(reinterpret_cast<const char*>(&lCount), sizeof(lCount));
            if (lCount > 0) {
                out.write(reinterpret_cast<const char*>(mesh->layout.data()), lCount * sizeof(InputLayout));
            }
        }

        // 3. Save Node Hierarchy
        SaveNode(root, out, meshMap);

        out.close();
        std::cout << "Saved scene to cache: " << path << std::endl;
    }

    GameObject* LoadNode(Scene* scene, GameObject* parent, std::ifstream& in, const std::vector<ResourceHandle<Mesh>>& meshes) {
        // Name
        std::string name = ReadString(in);
        GameObject* go = scene->CreateGameObject(name);
```
...
```cpp

        for (size_t i = 0; i < childCount; i++) {
            LoadNode(scene, go, in, meshes);
        }

        return go;
    }

    Scene* BistroSceneLoader::LoadFromCache(const std::string& path) {
```
...
```cpp

        Scene* newScene = new Scene("BistroScene");
        SceneManager::GetInstance()->SetCurrentScene(newScene);

        // 1. Load Meshes
        size_t meshCount;
        in.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
        std::vector<ResourceHandle<Mesh>> loadedMeshes;
        loadedMeshes.reserve(meshCount);

        for (size_t i = 0; i < meshCount; i++) {
            ResourceHandle<Mesh> handle = ResourceManager::GetInstance()->CreateResource<Mesh>();
            Mesh* mesh = handle.Get();

            // Bounds
            in.read(reinterpret_cast<char*>(&mesh->bounds), sizeof(AABB));

            // Vertices
            size_t vCount;
            in.read(reinterpret_cast<char*>(&vCount), sizeof(vCount));
            if (vCount > 0) {
                mesh->vertex.resize(vCount);
                in.read(reinterpret_cast<char*>(mesh->vertex.data()), vCount * sizeof(Vertex));
            }

            // Indices
            size_t iCount;
            in.read(reinterpret_cast<char*>(&iCount), sizeof(iCount));
            if (iCount > 0) {
                mesh->index.resize(iCount);
                in.read(reinterpret_cast<char*>(mesh->index.data()), iCount * sizeof(int));
            }

            // Layout
            size_t lCount;
            in.read(reinterpret_cast<char*>(&lCount), sizeof(lCount));
            if (lCount > 0) {
                mesh->layout.resize(lCount);
                in.read(reinterpret_cast<char*>(mesh->layout.data()), lCount * sizeof(InputLayout));
            }

            mesh->UploadMeshToGPU();
            loadedMeshes.push_back(handle);
        }
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

            Scene* scene = new Scene();
            SceneManager::GetInstance()->SetCurrentScene(scene);
            std::vector<SceneSerializedNode> allnode;
            StreamHelper::ReadVector(in, allnode);
            std::unordered_map<uint32_t, GameObject*> gameObjectMap;
            
            for(int i = 0; i < allnode.size(); i++)
            {
                auto& nodeData = allnode[i];
                
                std::string nodeName = nodeData.name;
                GameObject* go = scene->CreateGameObject(nodeName.empty() ? "Node" : nodeName);
                if(nodeData.parentIndex != -1)
                {
                    ASSERT(gameObjectMap.count(nodeData.parentIndex) > 0);
                    go->SetParent(gameObjectMap[nodeData.parentIndex]);
                }
                go->transform->SetLocalPosition(nodeData.position);
                go->transform->SetLocalQuaternion(nodeData.rotation);
                go->transform->SetLocalScale(nodeData.scale);

                gameObjectMap[i] = go;

                //todo 加入材质的异步加载：
                if(nodeData.meshID != 0)
                {
                    MeshFilter* filter = go->AddComponent<MeshFilter>();
                    filter->mMeshHandle = ResourceManager::GetInstance()->LoadAssetAsync<Mesh>(nodeData.meshID, [=]() 
                        {
                            filter->OnLoadResourceFinished();
                        }, nullptr);
                    
                    // 加载并设置 Material
                    if(nodeData.materialID != 0)
                    {
                        MeshRenderer* renderer = go->AddComponent<MeshRenderer>();
                        string path = AssetRegistry::GetInstance()->GetAssetPathFromID(nodeData.materialID);
                        ResourceHandle<Material> handle = ResourceManager::GetInstance()->LoadAssetAsync<Material>
                            ( nodeData.materialID,
                                [=]() 
                                {
                                    renderer->OnLoadResourceFinished();
                                }
                            ,nullptr
                            );
                        renderer->SetSharedMaterial(handle);
                    }
                }

            }
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

### File: `Runtime/Scene/Scene.h`
```cpp
namespace EngineCore
{
    class Scene : public Resource
    {
    public:
        Scene();
        ~Scene();
        Scene(const std::string& name):name(name){};
        void Open();
        void Close(){};
        void Update(uint32_t frameIndex);
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

        void PushNewTransformDirtyRoot(Transform* transform);

        
        //todo: 先用vector写死，后面要用priorityqueue之类的
        std::vector<Camera*> cameraStack;

        void RunLogicUpdate();
        void RunTransformUpdate();
        void RunRemoveInvalidDirtyRenderNode();

        uint32_t CreateRenderNode();
        
        void DeleteRenderNode(MeshRenderer *renderer);
        void MarkNodeCreated(MeshRenderer* renderer);
        void MarkNodeTransformDirty(Transform* transform);
        void MarkNodeMeshFilterDirty(MeshFilter* meshFilter);
        void MarkNodeMeshRendererDirty(MeshRenderer* renderer);
        void MarkNodeRenderableDirty(GameObject* object);
        
        inline std::vector<uint32_t>& GetPerFrameDirtyNodeList(){ return mPerFrameDirtyNodeList;}
        inline std::vector<uint32_t>& GetNodeChangeFlagList(){ return mNodeChangeFlagList;}    
        inline std::vector<NodeDirtyPayload>& GetNodeDirtyPayloadList(){ return mNodeDirtyPayloadList;}    
    public:
        std::string name;
        std::vector<GameObject*> allObjList;
        std::vector<GameObject*> rootObjList;
        bool enabled = true;
        Camera* mainCamera = nullptr;

        std::vector<Transform*> dirtyRootDepthBucket[64];
        
    private:
        uint32_t mCurrentFrame = 0;
        void ApplyQueueNodeChange(uint32_t id, uint32_t flags, const NodeDirtyPayload& p);
        void InternalMarkNodeDeleted(MeshRenderer* renderer);
        
        std::vector<uint32_t> mNodeFrameStampList;
        std::vector<uint32_t> mNodeChangeFlagList;
        std::vector<NodeDirtyPayload> mNodeDirtyPayloadList;


        std::vector<uint32_t> mPerFrameDirtyNodeList;
        
        uint32_t mCurrSceneIndex = 0;
        std::vector<uint32_t> mFreeSceneIndex;
        std::vector<uint32_t> mPendingFreeSceneIndex;

        void EnsureNodeQueueSize(uint32_t size);
        void ClearPerFrameData();
        void ClearDirtyRootTransform();
        void PushLastFrameFreeIndex();
    };    
} // namespace EngineCore
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

        void SetSharedMaterial(const ResourceHandle<Material>& mat);

        // return a new Material Instance;
        Material* GetOrCreateMatInstance();
        // 
        ResourceHandle<Material> GetMaterial();
        inline bool HasMaterialOverride() { return mInstanceMatHandler.IsValid(); }

        void UpdateBounds(const AABB& localBounds, const Matrix4x4& worldMatrix);

        bool shouldUpdateMeshRenderer = true;
        AABB worldBounds;
        bool materialDirty = true;
		
        uint32_t renderLayer = 1;
        void OnLoadResourceFinished();
        inline uint32_t GetCPUWorldIndex() { return mCPUWorldIndex;}
        inline void SetCPUWorldIndex(uint32_t index) { mCPUWorldIndex = index;}
    private:
        ResourceHandle<Material> mShardMatHandler;
        ResourceHandle<Material> mInstanceMatHandler;

        uint32_t mCPUWorldIndex = UINT32_MAX;
    };
```

### File: `Runtime/GameObject/Transform.h`
```cpp
{
    class GameObject;
    struct Transform : Component
    {
        Transform();
        Transform(GameObject* parent);
        virtual ~Transform() override;
        static ComponentType GetStaticType() { return ComponentType::Transform; };
        virtual ComponentType GetType() const override{ return ComponentType::Transform; };
        void MarkDirty();


        void RotateX(float degree);
        void RotateY(float degree);
        void RotateZ(float degree);

        const Vector3 GetLocalEulerAngles(); 
        void SetLocalEulerAngles(const Vector3& eulerAngles);

        const Vector3 GetWorldPosition(){ return mWorldPosition; };
        const Quaternion GetWorldQuaternion(){ return mWorldQuaternion; };
        const Vector3 GetWorldScale(){ return mWorldScale; };

        const Vector3 GetLocalPosition() const { return mLocalPosition; };
        const Quaternion GetLocalQuaternion() const { return mLocalQuaternion; };
        const Vector3 GetLocalScale() const { return mLocalScale; };

        void SetLocalPosition(const Vector3& localPosition);
        void SetLocalQuaternion(const Quaternion& localQuaternion);
        void SetLocalScale(const Vector3& localScale);

        inline void SetWorldPosition(const Vector3& position) { mWorldPosition = position; }
        inline void SetWorldQuaternion(const Quaternion& quaternion) { mWorldQuaternion = quaternion; }
        inline void SetWorldScale(const Vector3& scale) { mWorldScale = scale; }

        inline const Matrix4x4& GetWorldMatrix()
        {
            return mWorldMatrix;
        }
        
        inline const Matrix4x4& GetLocalMatrix()
        {
            return mLocalMatrix;
        }

        void UpdateRecursively(uint32_t frameID);
        inline uint32_t GetNodeDepth() { return mDepth; }

    public:
        bool isDirty = false;
        std::vector<Transform*> childTransforms;
        Transform* parentTransform = nullptr;
        
    protected:

        friend class GameObject;
        // 外部不能访问修改， 只能访问GameObject.SetParent
        inline void SetParent(Transform* transform)
        {
            ASSERT(transform);
            if(mDepth != 0)
            {
                parentTransform->RemoveChild(this);
            }
            parentTransform = transform; 
            transform->AddChild(this);
            mDepth = transform->GetNodeDepth() + 1;
            MarkDirty();
        };

        inline void DettachParent()
        {
            if(parentTransform != nullptr) parentTransform->RemoveChild(this);
            parentTransform = nullptr;
            mDepth = 0;
            MarkDirty();
        }

        inline void AddChild(Transform* transform)
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

### File: `Runtime/GameObject/Camera.h`
```cpp
namespace EngineCore
{
    class Camera : public Component
    {
    public:
        Camera() = default;
        Camera(GameObject* parent);
        virtual ~Camera() override {};
        float mFov;
        float mAspect;
        float mNear;
        float mFar; 
        float mWidth;
        float mHeight;
        Matrix4x4 mProjectionMatrix;
        Vector3 mLookAt;
        Matrix4x4 mViewMatrix;
        Frustum mFrustum;

        static ComponentType GetStaticType() { return ComponentType::Camera; };
        virtual ComponentType GetType() const override { return ComponentType::Camera; };
        void Update();
    public:
        RenderPassAsset mRenderPassAsset;
        // 这两个只是描述符， 没必要用指针
        RenderTexture* depthAttachment;
        RenderTexture* colorAttachment;
        void UpdateCameraMatrix();


        virtual const char* GetScriptName() const override { return "Camera"; }
    };
```

### File: `Runtime/GameObject/MonoBehaviour.h`
```cpp
{
    class GameObject;
    class MonoBehaviour : public Component
    {
    public:
        MonoBehaviour() = default;
        MonoBehaviour(GameObject* parent);
        virtual ~MonoBehaviour() = default;
        // 非纯虚， 不一定要实现
        virtual void Awake() {};
        virtual void Start() {};
        virtual void Update() {};
        virtual void LateUpdate() {};
        virtual void OnDestroy() {};
        static ComponentType GetStaticType() { return ComponentType::Script; }
        virtual ComponentType GetType() const { return ComponentType::Script; };
        
        // 每个类需要自己实现序列化和反序列化方法。
        virtual const char* GetScriptName() const = 0;
    protected:


    };
```

### File: `Runtime/Scene/SceneStruct.h`
```cpp
        NodeDirtyPayload() = default;
        NodeDirtyPayload(Transform* trans)
            : transform(trans){}
```
...
```cpp
    };

    struct GPUSceneDelta
    {
        vector<uint32_t> dirtyRenderNodeIDList;
        vector<uint32_t> dirtyRenderNodeFlagsList;
    };
```

### File: `Runtime/Scene/CPUScene.h`
```cpp
namespace EngineCore
{
    class CPUScene
    {
    public:
        void Update(uint32_t frameID);
      
        void ApplyDirtyNode(uint32_t renderID, NodeDirtyFlags cpuWorldRenderNodeFlag , NodeDirtyPayload& payload);
        void EndFrame();
        CPUSceneView GetSceneView();

    private:
        void EnsureCapacity(uint32_t renderID);
        void CreateRenderNode(uint32_t renderID, NodeDirtyPayload& payload);
        void DeleteRenderNode(uint32_t renderID); 
        void OnRenderNodeMaterialDirty(uint32_t renderID, NodeDirtyPayload& payload);
        void OnRenderNodeTransformDirty(uint32_t renderID, NodeDirtyPayload& payload);
        void OnRenderNodeMeshDirty(uint32_t renderID, NodeDirtyPayload& payload);
    
        
    private:
        vector<AssetID> materialList;
        vector<AssetID> meshList;
        vector<AABB> worldBoundsList;
        vector<AABB> localBoundCacheList;
        vector<Matrix4x4> objectToWorldMatrixList;
        vector<uint32_t> layerList;

        uint32_t mCurrentFrame = 0;
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
        static void Update(uint32_t frameIndex);
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

### File: `Runtime/GameObject/Component.h`
```cpp
{
    class GameObject;
    class Component
    {
    public:
        Component(){};
        virtual ~Component() = 0;
        virtual ComponentType GetType() const = 0;
        
        GameObject* gameObject = nullptr;
        bool enabled = true;
        
        // 每个类需要自己实现序列化和反序列化方法。
        virtual const char* GetScriptName() const = 0;
    };
```