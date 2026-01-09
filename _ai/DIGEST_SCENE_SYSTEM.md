# Architecture Digest: SCENE_SYSTEM
> Auto-generated. Focus: Runtime/Scene, SceneManager, Scene, GameObject, Component, Transform, Hierarchy

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- Scene与GameObject/Component的组织结构。
- 场景管理、加载与切换流程。

## Key Files Index
- `[68]` **Runtime/Scene/SceneManager.h** *(Content Included)*
- `[51]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[49]` **Runtime/GameObject/Transform.h** *(Content Included)*
- `[48]` **Runtime/Scene/Scene.h** *(Content Included)*
- `[42]` **Runtime/GameObject/GameObject.h** *(Content Included)*
- `[40]` **Runtime/GameObject/Component.h** *(Content Included)*
- `[37]` **Runtime/GameObject/ComponentType.h** *(Content Included)*
- `[35]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h** *(Content Included)*
- `[31]` **Runtime/Serialization/ComponentFactory.h** *(Content Included)*
- `[29]` **Editor/Panel/EditorHierarchyPanel.h** *(Content Included)*
- `[24]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[22]` **Runtime/GameObject/MeshRenderer.h**
- `[19]` **Runtime/Serialization/MetaFactory.h**
- `[19]` **Runtime/GameObject/MonoBehaviour.h**
- `[18]` **Runtime/GameObject/Camera.h**
- `[18]` **Runtime/GameObject/MeshFilter.h**
- `[13]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[11]` **Editor/Panel/EditorMainBar.h**
- `[9]` **Editor/Panel/EditorInspectorPanel.h**
- `[8]` **Runtime/Graphics/ModelData.h**
- `[7]` **Editor/EditorSettings.h**
- `[7]` **Editor/EditorGUIManager.h**
- `[5]` **Runtime/EngineCore.h**
- `[5]` **Runtime/Renderer/RenderEngine.h**
- `[4]` **Runtime/Scripts/CameraController.h**
- `[4]` **Runtime/Platforms/D3D12/D3D12RootSignature.h**
- `[4]` **Runtime/Math/AABB.h**
- `[4]` **Runtime/Renderer/BatchManager.h**
- `[4]` **Runtime/Renderer/RenderSorter.h**
- `[3]` **Runtime/Graphics/ModelUtils.h**
- `[3]` **Runtime/Core/PublicStruct.h**
- `[3]` **Runtime/Renderer/RenderContext.h**
- `[3]` **Runtime/Renderer/Culling.h**
- `[3]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[2]` **Runtime/CoreAssert.h**
- `[2]` **Runtime/PreCompiledHeader.h**
- `[2]` **Runtime/Graphics/MaterialLayout.h**
- `[2]` **Runtime/Graphics/Texture.h**
- `[2]` **Runtime/Graphics/ComputeShader.h**
- `[2]` **Runtime/Graphics/IGPUResource.h**
- `[2]` **Runtime/Graphics/GPUTexture.h**
- `[2]` **Runtime/Graphics/RenderTexture.h**
- `[2]` **Runtime/Graphics/GPUBufferAllocator.h**
- `[2]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[2]` **Runtime/Graphics/Material.h**
- `[2]` **Runtime/Graphics/Shader.h**
- `[2]` **Runtime/Graphics/MaterialInstance.h**
- `[2]` **Runtime/Resources/Resource.h**
- `[2]` **Runtime/Resources/ResourceHandle.h**
- `[2]` **Runtime/Resources/Asset.h**
- `[2]` **Runtime/Resources/ResourceManager.h**
- `[2]` **Runtime/Serialization/JsonSerializer.h**
- `[2]` **Runtime/Serialization/BaseTypeSerialization.h**
- `[2]` **Runtime/Serialization/MetaData.h**
- `[2]` **Runtime/Serialization/AssetSerialization.h**
- `[2]` **Runtime/Serialization/MetaLoader.h**
- `[2]` **Runtime/Core/PublicEnum.h**
- `[2]` **Runtime/Core/Profiler.h**
- `[2]` **Runtime/Core/InstanceID.h**
- `[2]` **Runtime/Core/Object.h**

## Evidence & Implementation Details

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
        ModelData* quadMesh;
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

        inline const Matrix4x4& GetWorldMatrix()
        {
            UpdateIfDirty(); 
            return mWorldMatrix;
        }
        
        inline const Matrix4x4& GetLocalMatrix()
        {
            UpdateIfDirty(); 
            return mLocalMatrix;
        }

        void UpdateIfDirty();
        void UpdateTransform();
        //inline void UpdateNow() { UpdateTransform(); };
        uint32_t transformVersion = 1;
    public:
        bool isDirty = true;
        std::vector<Transform*> childTransforms;
        Transform* parentTransform = nullptr;
        
    protected:

        friend class GameObject;
        // 外部不能访问修改， 只能访问GameObject.SetParent
        inline void SetParent(Transform* transform)
        {
            parentTransform = transform; 
```
...
```cpp
        inline void DettachParent()
        {
            if(parentTransform != nullptr) parentTransform->RemoveChild(this);
            parentTransform = nullptr;
        }

        inline void AddChild(Transform* transform)
        {
```

### File: `Runtime/Scene/Scene.h`
```cpp
namespace EngineCore
{
    struct RenderSceneData
    {
        vector<MeshRenderer*> meshRendererList;
        vector<uint32_t> vaoIDList;
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
                    vaoIDList[index] = meshFilter->mMeshHandle.Get()->GetInstanceID();
                }
                else
                {
                    vaoIDList[index] = UINT32_MAX;
                }
            }
        }

        inline void PushNewData() 
        {
            meshRendererList.emplace_back();
            vaoIDList.emplace_back();
            aabbList.emplace_back();
            objectToWorldMatrixList.emplace_back();
            layerList.emplace_back();
        }


        inline void DeleteData(uint32_t index)
        {
            meshRendererList[index] = nullptr;
            vaoIDList[index] = UINT32_MAX;

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
    private:
        Scene* ownerScene = nullptr;
    };
```

### File: `Runtime/GameObject/Component.h`
```cpp
    using json = nlohmann::json;

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
        virtual json SerializedFields() const { return json::object(); }
        virtual void DeserializedFields(const json& j) {};
    };
```

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
```

### File: `Runtime/Serialization/ComponentFactory.h`
```cpp
namespace EngineCore
{
    class ComponentFactory
    {
    public :
        using CreateFunc = std::function<Component*(GameObject*)>;
        inline static Component* Create(const std::string& componentName, GameObject* go)
        {
            auto& registry = GetRegistry();
            if(registry.count(componentName) > 0)
            {
                return registry[componentName](go);
            }

            ASSERT_MSG(false, "Can't find this script");
            return nullptr;
        }

        inline static void Register(const std::string& componentName, CreateFunc createFunc)
        {
            GetRegistry()[componentName] = createFunc;
        }
    private :

        // 这样写的好处， 不用在cpp调用， 初次调用的时候创建，一定保证时序
        static std::unordered_map<std::string, CreateFunc>& GetRegistry()
        {
            static std::unordered_map<std::string, CreateFunc> registry;
            return registry;
        }
    };
```
...
```cpp
#define REGISTER_SCRIPT(ComponentClass)\
    namespace { \
        struct ComponentClass##_Register { \
            ComponentClass##_Register() { \
                EngineCore::ComponentFactory::Register( #ComponentClass, \
                    [](EngineCore::GameObject* go) -> EngineCore::Component* {\
                        return new EngineCore::ComponentClass(go); \
                    }); \
            } \
        } ComponentClass##_instance; \
```

### File: `Editor/Panel/EditorHierarchyPanel.h`
```cpp
{
    using EngineCore::GameObject;
    class EditorHierarchyPanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorHierarchyPanel() override;  
    private:
        int nodeIdx = 0;
        EngineCore::GameObject* selectedGO = nullptr;
        ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | 
            ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;   
        void DrawNode(GameObject* go);   
    }; 
```