# Architecture Digest: GAMEPLAY
> Auto-generated. Focus: Runtime/GameObject, Runtime/Scene, Runtime/Scripts, Runtime/Managers, Window, Input, Camera, Transform, Component, MonoBehaviour

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 围绕GameObject、Component、Transform、Camera与Scene的生命周期。
- 关注运行时脚本与管理器如何驱动渲染与资源依赖。

## Key Files Index
- `[42]` **Runtime/GameObject/Camera.h** *(Content Included)*
- `[42]` **Runtime/GameObject/Transform.h** *(Content Included)*
- `[41]` **Runtime/GameObject/MonoBehaviour.h** *(Content Included)*
- `[38]` **Runtime/GameObject/ComponentType.h** *(Content Included)*
- `[38]` **Runtime/Scripts/CameraController.h** *(Content Included)*
- `[37]` **Runtime/GameObject/Component.h** *(Content Included)*
- `[37]` **Runtime/Managers/WindowManager.h** *(Content Included)*
- `[27]` **Runtime/Platforms/Windows/WindowManagerWindows.h** *(Content Included)*
- `[26]` **Runtime/GameObject/GameObject.h** *(Content Included)*
- `[23]` **Runtime/Scene/Scene.h** *(Content Included)*
- `[19]` **Runtime/GameObject/MeshRenderer.h**
- `[17]` **Runtime/GameObject/MeshFilter.h**
- `[14]` **Runtime/Scene/SceneManager.h**
- `[12]` **Runtime/Managers/Manager.h**
- `[12]` **Runtime/Scene/BistroSceneLoader.h**
- `[9]` **Runtime/Serialization/SceneLoader.h**
- `[8]` **Runtime/Graphics/Mesh.h**
- `[8]` **Editor/Panel/EditorInspectorPanel.h**
- `[7]` **Editor/EditorSettings.h**
- `[7]` **Runtime/PreCompiledHeader.h**
- `[7]` **Runtime/Renderer/RenderSorter.h**
- `[7]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[6]` **Runtime/Renderer/RenderCommand.h**
- `[6]` **Runtime/Renderer/RenderContext.h**
- `[6]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[6]` **Assets/Shader/SimpleTestShader.hlsl**
- `[6]` **Assets/Shader/StandardPBR.hlsl**
- `[6]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[5]` **Runtime/Renderer/RenderAPI.h**
- `[5]` **Runtime/Renderer/Renderer.h**
- `[5]` **Runtime/Settings/ProjectSettings.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[5]` **Assets/Shader/BlitShader.hlsl**
- `[5]` **Assets/Shader/GPUCulling.hlsl**
- `[4]` **premake5.lua**
- `[4]` **Runtime/Core/PublicStruct.h**
- `[4]` **Runtime/Graphics/Shader.h**
- `[4]` **Runtime/Math/AABB.h**
- `[4]` **Runtime/Renderer/Culling.h**
- `[4]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[4]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[3]` **Runtime/EngineCore.h**
- `[3]` **Runtime/Graphics/GPUSceneManager.h**
- `[3]` **Runtime/Renderer/RenderUniforms.h**
- `[3]` **Editor/D3D12/D3D12EditorGUIManager.h**
- `[2]` **Editor/EditorGUIManager.h**
- `[2]` **Runtime/CoreAssert.h**
- `[2]` **Runtime/Core/Game.h**
- `[2]` **Runtime/Core/InstanceID.h**
- `[2]` **Runtime/Core/Object.h**
- `[2]` **Runtime/Core/Profiler.h**
- `[2]` **Runtime/Core/PublicEnum.h**
- `[2]` **Runtime/Core/ThreadSafeQueue.h**
- `[2]` **Runtime/Graphics/ComputeShader.h**
- `[2]` **Runtime/Graphics/GeometryManager.h**
- `[2]` **Runtime/Graphics/GPUBufferAllocator.h**
- `[2]` **Runtime/Graphics/GPUTexture.h**
- `[2]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[2]` **Runtime/Graphics/IGPUResource.h**
- `[2]` **Runtime/Graphics/Material.h**

## Evidence & Implementation Details

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
```
...
```cpp
        {
            parentTransform = transform; 
            if(transform)transform->AddChild(this);
        };

        inline void DettachParent()
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
    class  WindowManager : public Manager<WindowManager>
    {
    public:
        static void Update();
        static void Create();
        virtual bool WindowShouldClose() = 0;
        virtual void OnResize() = 0;
        virtual void* GetWindow() = 0;
        inline int GetWidth() { return mWindowWidth; };
        inline int GetHeight() { return mWindowHeight; };
        std::pair<int, int> GetWindowSize()
        {
            return {mWindowWidth, mWindowHeight};
        }
        ~WindowManager() override {};
        WindowManager(){};
    protected:
        int mWindowWidth;
        int mWindowHeight;
        bool mMinimized = false;
        bool mResizing = false;
        bool mResized = false;
        bool mAppPaused = false;
		bool mMaximized = false;
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
        static void Create();
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