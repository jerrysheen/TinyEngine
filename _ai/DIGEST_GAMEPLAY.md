# Architecture Digest: GAMEPLAY
> Auto-generated. Focus: Runtime/GameObject, Runtime/Scene, Runtime/Scripts, Runtime/Managers, Window, Input, Camera, Transform, Component, MonoBehaviour

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 围绕GameObject、Component、Transform、Camera与Scene的生命周期。
- 关注运行时脚本与管理器如何驱动渲染与资源依赖。

## Key Files Index
- `[42]` **Runtime/GameObject/Camera.cpp** *(Content Included)*
- `[42]` **Runtime/GameObject/Camera.h** *(Content Included)*
- `[42]` **Runtime/GameObject/Transform.h** *(Content Included)*
- `[41]` **Runtime/GameObject/MonoBehaviour.h** *(Content Included)*
- `[38]` **Runtime/GameObject/ComponentType.h** *(Content Included)*
- `[38]` **Runtime/Scripts/CameraController.h** *(Content Included)*
- `[37]` **Runtime/GameObject/Component.h** *(Content Included)*
- `[37]` **Runtime/Managers/WindowManager.h** *(Content Included)*
- `[35]` **Runtime/GameObject/Transform.cpp** *(Content Included)*
- `[35]` **Runtime/Managers/WindowManager.cpp** *(Content Included)*
- `[35]` **Runtime/Scripts/CameraController.cpp** *(Content Included)*
- `[33]` **Runtime/GameObject/Component.cpp** *(Content Included)*
- `[33]` **Runtime/GameObject/MonoBehaviour.cpp** *(Content Included)*
- `[27]` **Runtime/Platforms/Windows/WindowManagerWindows.h** *(Content Included)*
- `[26]` **Runtime/GameObject/GameObject.h** *(Content Included)*
- `[25]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[25]` **Runtime/Scene/Scene.cpp** *(Content Included)*
- `[25]` **Runtime/Platforms/Windows/WindowManagerWindows.cpp** *(Content Included)*
- `[24]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[22]` **Runtime/Scene/Scene.h** *(Content Included)*
- `[21]` **Runtime/GameObject/GameObject.cpp**
- `[20]` **Runtime/Entry.cpp**
- `[18]` **Runtime/GameObject/MeshRenderer.h**
- `[17]` **Runtime/GameObject/MeshFilter.h**
- `[17]` **Runtime/Scene/SceneStruct.h**
- `[15]` **Runtime/Scene/CPUScene.cpp**
- `[14]` **Runtime/Core/Game.cpp**
- `[14]` **Runtime/Scene/SceneManager.h**
- `[13]` **Runtime/Renderer/RenderEngine.cpp**
- `[13]` **Runtime/Scene/CPUScene.h**
- `[13]` **Editor/Panel/EditorMainBar.cpp**
- `[12]` **Runtime/Managers/Manager.h**
- `[12]` **Runtime/Scene/BistroSceneLoader.h**
- `[12]` **Runtime/Scene/GPUScene.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[12]` **Editor/Panel/EditorInspectorPanel.cpp**
- `[11]` **Runtime/GameObject/MeshRenderer.cpp**
- `[11]` **Runtime/Serialization/SceneLoader.h**
- `[11]` **Assets/Shader/StandardPBR.hlsl**
- `[10]` **Runtime/GameObject/MeshFilter.cpp**
- `[10]` **Runtime/Scene/GPUScene.cpp**
- `[10]` **Assets/Shader/SimpleTestShader.hlsl**
- `[10]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[9]` **Runtime/Renderer/Renderer.cpp**
- `[9]` **Runtime/Renderer/Renderer.h**
- `[9]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[9]` **Assets/Shader/BlitShader.hlsl**
- `[9]` **Assets/Shader/GPUCulling.hlsl**
- `[8]` **Editor/EditorSettings.cpp**
- `[8]` **Runtime/Graphics/Mesh.h**
- `[8]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[8]` **Editor/Panel/EditorHierarchyPanel.cpp**
- `[8]` **Editor/Panel/EditorInspectorPanel.h**
- `[7]` **Editor/EditorSettings.h**
- `[7]` **Runtime/PreCompiledHeader.h**
- `[7]` **Runtime/Renderer/RenderSorter.h**
- `[6]` **Runtime/Renderer/RenderCommand.h**
- `[6]` **Runtime/Renderer/RenderContext.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[5]` **Runtime/Graphics/Mesh.cpp**

## Evidence & Implementation Details

### File: `Runtime/GameObject/Camera.cpp`
```cpp
        //  0, 0, -0.003, 0.29,
        //  0.53, -0.40, 0.74, 4.5);
        UpdateCameraMatrix();
    }

    // 物体已经在世界坐标系中， 所以只需要用vp矩阵判断， Mvp * p;
    void Camera::Update()
    {
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

### File: `Runtime/Scripts/CameraController.h`
```cpp
namespace EngineCore
{
    class CameraController : public MonoBehaviour
    {
    public:
        CameraController() = default;
        CameraController(GameObject* go);
        virtual ~CameraController() override {};

        virtual void Update() override;
        virtual const char* GetScriptName() const override;
    public:
        float testVal = 0;
    };
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

### File: `Runtime/Managers/WindowManager.h`
```cpp
namespace EngineCore
{
    class  WindowManager
    {
    public:
        inline static WindowManager* GetInstance() 
        {
            if (s_Instance == nullptr)
            {
                Create();
            };
            return s_Instance.get();
        }
        static void Update();
        static void Create();
        static void Destroy();
        virtual bool WindowShouldClose() = 0;
        virtual void OnResize() = 0;
        virtual void* GetWindow() = 0;
        inline int GetWidth() { return mWindowWidth; };
        inline int GetHeight() { return mWindowHeight; };
        std::pair<int, int> GetWindowSize()
        {
            return {mWindowWidth, mWindowHeight};
        }
        ~WindowManager(){};
        WindowManager(){};

    protected:
        int mWindowWidth;
        int mWindowHeight;
        bool mMinimized = false;
        bool mResizing = false;
        bool mResized = false;
        bool mAppPaused = false;
		bool mMaximized = false;
        static std::unique_ptr<WindowManager> s_Instance;
    };
```

### File: `Runtime/Platforms/Windows/WindowManagerWindows.h`
```cpp
namespace EngineCore
{
    class WindowManagerWindows : public WindowManager
    {
    public :
        static void Update();
        void Create();
        virtual bool WindowShouldClose() override;
        virtual void OnResize() override;
        WindowManagerWindows();
        virtual void* GetWindow() override {return mWindow;}
        LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        void Show();
    private:
        void InitializeWindowsWindow();
        HWND mWindow;
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
        inline Scene* GetOwnerScene() { return ownerScene; }
        inline void SetOwnerScene(Scene* scene) { ownerScene = scene; }
    private:
        Scene* ownerScene = nullptr;
    };
```

### File: `Runtime/Scene/Scene.h`
```cpp
        void Scene::DestroyGameObject(const std::string& name);

        void AddCamToStack(Camera* cam);
        inline void SetMainCamera(Camera* cam) { mainCamera = cam; }
```
...
```cpp
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
```
...
```cpp
        void EnsureNodeQueueSize(uint32_t size);
        void ClearPerFrameData();
        void ClearDirtyRootTransform();
        void PushLastFrameFreeIndex();
    };    
} // namespace EngineCore
```