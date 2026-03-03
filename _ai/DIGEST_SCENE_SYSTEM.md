# Architecture Digest: SCENE_SYSTEM
> Auto-generated. Focus: Runtime/Scene, Runtime/Scene/SceneStruct.h, SceneManager, Scene, SceneLoader, BistroSceneLoader, GameObject, Component, Transform, Hierarchy, MeshFilter, MeshRenderer, CPUScene, GPUScene, NodeDirtyFlags, NodeDirtyPayload, dirtyRootDepthBucket, mPerFrameDirtyNodeList, MarkNodeTransformDirty, MarkNodeMeshFilterDirty, MarkNodeMeshRendererDirty, MarkNodeRenderableDirty, Game

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
- Scene与GameObject/Component的组织结构。
- 场景管理、加载与切换流程，关注Transform分层更新与DirtyList汇聚（dirtyRootDepthBucket、mPerFrameDirtyNodeList）。

## Key Files Index
- `[120]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[91]` **Runtime/Scene/BistroSceneLoader.h** *(Content Included)*
- `[88]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[87]` **Runtime/Scene/Scene.cpp** *(Content Included)*
- `[81]` **Runtime/Scene/CPUScene.cpp** *(Content Included)*
- `[79]` **Runtime/Serialization/SceneLoader.h** *(Content Included)*
- `[74]` **Runtime/Scene/SceneManager.h** *(Content Included)*
- `[72]` **Runtime/Scene/CPUScene.h** *(Content Included)*
- `[72]` **Runtime/Scene/Scene.h** *(Content Included)*
- `[69]` **Runtime/GameObject/GameObject.h** *(Content Included)*
- `[67]` **Runtime/GameObject/GameObject.cpp** *(Content Included)*
- `[65]` **Runtime/GameObject/MeshRenderer.cpp** *(Content Included)*
- `[64]` **Runtime/Scene/GPUScene.cpp** *(Content Included)*
- `[63]` **Runtime/GameObject/Transform.h** *(Content Included)*
- `[62]` **Runtime/GameObject/MeshFilter.cpp** *(Content Included)*
- `[62]` **Runtime/Scene/GPUScene.h** *(Content Included)*
- `[62]` **Runtime/Scene/SceneStruct.h** *(Content Included)*
- `[61]` **Runtime/GameObject/Transform.cpp** *(Content Included)*
- `[58]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[58]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.cpp** *(Content Included)*
- `[55]` **Runtime/GameObject/MeshFilter.h**
- `[53]` **Runtime/GameObject/Component.h**
- `[51]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[50]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[49]` **Runtime/GameObject/ComponentType.h**
- `[48]` **Runtime/Core/Game.cpp**
- `[46]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[46]` **Editor/Panel/EditorHierarchyPanel.cpp**
- `[45]` **Runtime/GameObject/Component.cpp**
- `[43]` **Runtime/GameObject/Camera.cpp**
- `[40]` **Editor/Panel/EditorMainBar.cpp**
- `[35]` **Runtime/Renderer/RenderEngine.cpp**
- `[34]` **Editor/Panel/EditorHierarchyPanel.h**
- `[31]` **Runtime/GameObject/MonoBehaviour.h**
- `[29]` **Runtime/GameObject/Camera.h**
- `[28]` **Editor/Panel/EditorGameViewPanel.cpp**
- `[27]` **Runtime/Core/Game.h**
- `[26]` **Runtime/GameObject/MonoBehaviour.cpp**
- `[24]` **Runtime/Entry.cpp**
- `[24]` **Editor/Panel/EditorGameViewPanel.h**
- `[22]` **Runtime/Renderer/Culling.cpp**
- `[18]` **Runtime/Renderer/FrameContext.cpp**
- `[17]` **Runtime/Renderer/RenderEngine.h**
- `[14]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[13]` **Editor/EditorSettings.cpp**
- `[13]` **Runtime/Renderer/RenderContext.cpp**
- `[12]` **Editor/EditorGUIManager.h**
- `[12]` **Editor/EditorSettings.h**
- `[12]` **Editor/Panel/EditorMainBar.h**
- `[11]` **Runtime/Renderer/FrameContext.h**
- `[10]` **Runtime/Graphics/Material.cpp**
- `[10]` **Editor/Panel/EditorInspectorPanel.h**
- `[9]` **Runtime/Renderer/BatchManager.h**
- `[9]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[9]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[9]` **Editor/Panel/EditorInspectorPanel.cpp**
- `[8]` **Runtime/Graphics/Mesh.h**
- `[8]` **Runtime/Scripts/CameraController.cpp**
- `[7]` **Runtime/Renderer/Renderer.h**
- `[6]` **Editor/EditorGUIManager.cpp**

## Evidence & Implementation Details

### File: `Runtime/Scene/BistroSceneLoader.cpp`
```cpp
            Scene* res = static_cast<Scene*>(sceneLoader.Load(binPath).resource);
            // todo： 这个地方有加载时序问题。。
            SceneManager::GetInstance()->SetCurrentScene(res);
            // 刷新所有Transform的world position，避免父子节点关系建立后的延迟更新问题
            //for (auto& gameObject : res->rootObjList) 
            //{
            //    if (gameObject != nullptr && gameObject->transform != nullptr)
            //    {
            //        gameObject->transform->UpdateRecursively(0);
            //    }
            //}
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

### File: `Runtime/Scene/BistroSceneLoader.h`
```cpp

namespace EngineCore {
    class GameObject;
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
        void ProcessShaders();
        void ProcessTexture(const tinygltf::Model& model);
        void CreateDefaultResources();
        void AttachMaterialToGameObject(GameObject* gameObject, int materialIndex);
        AssetID GetTextureAssetID(const tinygltf::Model& model, int textureIndex);
        std::map<int, std::vector<std::pair<ResourceHandle<Mesh>, int>>> m_MeshCache;
        std::vector<AssetID> m_ImageIndexToID;
        std::vector<ResourceHandle<Material>> m_MaterialMap;
    };
```

### File: `Runtime/Scene/SceneManager.cpp`
```cpp
    inline Scene* SceneManager::AddNewScene(const std::string& name)
    {
        ASSERT_MSG(mSceneMap.count(name) == 0, "The Same Name Scene Has Been Created");
        Scene* scene = new Scene(name);
        mSceneMap.try_emplace(name, scene);
        if (mCurrentScene == nullptr) 
        {
            mCurrentScene = scene;
        }
```
...
```cpp
    void SceneManager::RemoveScene(const std::string& name)
    {
        ASSERT_MSG(mSceneMap.count(name) > 0, "Can't find this scene");
        // auto* 和 auto 是一样的道理， *在这个地方其实是一种语义，表示指针
        auto* scene = mSceneMap[name];
        if(mCurrentScene == scene)
        {
            scene->Close();
        }
```
...
```cpp
    void SceneManager::SwitchSceneTo(const std::string& name)
    {
        ASSERT(mSceneMap.count(name) > 0);
        auto* scene = mSceneMap[name];
        if(scene == mCurrentScene) return;
        mCurrentScene->Close();
        scene->Open();
        mCurrentScene  = scene;
    };

    void SceneManager::Update(uint32_t frameIndex)
    {
```
...
```cpp
    void SceneManager::SetCurrentFrame(uint32_t currentFrameIndex)
    {
        if (mCurrentScene) mCurrentScene->SetCurrentFrame(currentFrameIndex);
    }

    void SceneManager::Create()
    {
```
...
```cpp


        ASSERT(scene);
        if (scene) {
             mSceneMap[scene->name] = scene;
             mCurrentScene = scene;
             scene->Open();
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
...
```cpp
            else
            {
                ASSERT(gameObjectMap.count(parent->gameObject) > 0);
                node.parentIndex = gameObjectMap[parent->gameObject];
                gameObjectMap[gameObject] = linearNode.size();
            }
            node.materialID = materialID;
            node.meshID = meshID;
            node.position = gameObject->transform->GetLocalPosition();
            node.rotation = gameObject->transform->GetLocalQuaternion();
            node.scale = gameObject->transform->GetLocalScale();
            linearNode.push_back(node);

            for (auto* child : gameObject->GetChildren()) 
            {
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
        void SetCurrentFrame(uint32_t currentFrameIndex);
    private:
        static SceneManager* s_Instance;
        Scene* mCurrentScene = nullptr;
        unordered_map<std::string, Scene*> mSceneMap;
        vector<ResourceHandle<Texture>> texHandler;
    };

}
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

        inline void SetCurrentFrame(uint32_t currentFrame)
        {   
            mCurrentFrame = currentFrame;
        }
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

        inline void SetCurrentFrame(uint32_t currentFrameIndex)
        {
            mCurrentFrame = currentFrameIndex;
        }
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
```

### File: `Runtime/GameObject/GameObject.h`
```cpp
namespace EngineCore
{
    class Component;
    class Scene;
    class Transform;
    //class Transform;
    // later we will inherete from Object, now just keep it simple.
    class GameObject : Object
    {
    public:
        GameObject();

        GameObject::GameObject(const std::string& name, Scene* scene);

        ~GameObject();
        void SetParent(const GameObject* gameObject);
        template<typename T>
        inline T* GetComponent() const;
        template<typename T>
        inline T* AddComponent();

        std::vector<GameObject*> GetChildren() const;
    public:
        Transform* transform;
        std::unordered_map<ComponentType, Component*> components;
        std::vector<MonoBehaviour*> scripts;
        std::string name;
        bool enabled = true;
            // 非模板方式
        void AddComponent(Component* compont);
        inline Scene* GetOwnerScene() { return ownerScene; }
        inline void SetOwnerScene(Scene* scene) { ownerScene = scene; }
    private:
        Scene* ownerScene = nullptr;
    };
```

### File: `Runtime/GameObject/Transform.h`
```cpp
namespace EngineCore
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

### File: `Runtime/Scene/GPUScene.h`
```cpp
{

    class GPUScene
    {
    public:
        GPUScene();
        void Create();
        void Update(uint32_t currentFrameIndex);
        void Destroy();
        void EndFrame();
        
        BufferAllocation GetSinglePerMaterialData();
        BufferAllocation UploadDrawBatch(void *data, uint32_t size);

        void UpdateDirtyNode(CPUSceneView& view);
        void UploadCopyOp();

        void ApplyDirtyNode(uint32_t renderID, uint32_t flags, CPUSceneView& view);


        void UpdateFrameContextDirtyFlags(uint32_t renderID, uint32_t flag);
        void UpdateFrameContextShadowData(uint32_t renderID, CPUSceneView& view);
        
        FrameContext* GetCurrentFrameContexts();
        inline uint32_t GetCurrentFrameID() const { return mCurrentFrameID; }

        inline GPUBufferAllocator* GetAllMaterialDataBuffer() { return allMaterialDataBuffer; }
        inline ResourceHandle<ComputeShader> GetCullingShaderHandler() { return GPUCullingShaderHandler; }

        inline void SetCurrentFrame(uint32_t currentFrame)
        {
            mCurrentFrameID = currentFrame;
        }
    private:
        void EnsureCapacity(uint32_t renderID);
    private:
        ResourceHandle<ComputeShader> GPUCullingShaderHandler;
        static const int mMaxFrameCount = 3;
        uint32_t mCurrentFrameID = 0;
        FrameContext* mCurrentFrameContext;
        FrameContext mCPUFrameContext[mMaxFrameCount];
        GPUBufferAllocator* allMaterialDataBuffer;
    };
```

### File: `Runtime/Scene/SceneStruct.h`
```cpp
{

    enum class NodeDirtyFlags : uint32_t
    {
        None = 0,
        Created = 1 << 0,
        Destory = 1 << 1,
        TransformDirty = 1 << 2,
        MeshDirty = 1 << 4,
        MaterialDirty = 1 << 5,
    };
```
...
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
        void SetDefaultMaterial();
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