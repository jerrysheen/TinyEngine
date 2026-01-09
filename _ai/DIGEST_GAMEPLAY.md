# Architecture Digest: GAMEPLAY
> Auto-generated. Focus: Runtime/GameObject, Runtime/Scene, Runtime/Scripts, Runtime/Managers, Window, Input, Camera, Transform, Component, MonoBehaviour

## Key Files Index
- `[43]` **Runtime/GameObject/Camera.cpp** *(Content Included)*
- `[42]` **Runtime/GameObject/Camera.h** *(Content Included)*
- `[42]` **Runtime/GameObject/Transform.h** *(Content Included)*
- `[41]` **Runtime/GameObject/MonoBehaviour.h** *(Content Included)*
- `[38]` **Runtime/GameObject/ComponentType.h** *(Content Included)*
- `[38]` **Runtime/Scripts/CameraController.h** *(Content Included)*
- `[37]` **Runtime/GameObject/Component.h** *(Content Included)*
- `[37]` **Runtime/Managers/WindowManager.h** *(Content Included)*
- `[36]` **Runtime/GameObject/Transform.cpp** *(Content Included)*
- `[36]` **Runtime/Scripts/CameraController.cpp** *(Content Included)*
- `[35]` **Runtime/Managers/WindowManager.cpp** *(Content Included)*
- `[33]` **Runtime/GameObject/Component.cpp** *(Content Included)*
- `[33]` **Runtime/GameObject/MonoBehaviour.cpp** *(Content Included)*
- `[27]` **Runtime/Serialization/ComponentFactory.h** *(Content Included)*
- `[27]` **Runtime/Platforms/Windows/WindowManagerWindows.h** *(Content Included)*
- `[26]` **Runtime/GameObject/GameObject.h** *(Content Included)*
- `[25]` **Runtime/Platforms/Windows/WindowManagerWindows.cpp** *(Content Included)*
- `[23]` **Runtime/Scene/Scene.cpp** *(Content Included)*
- `[23]` **Runtime/Scene/Scene.h** *(Content Included)*
- `[21]` **Runtime/GameObject/GameObject.cpp** *(Content Included)*
- `[19]` **Runtime/GameObject/MeshRenderer.h**
- `[17]` **Runtime/GameObject/MeshFilter.h**
- `[15]` **Runtime/GameObject/MeshRenderer.cpp**
- `[14]` **Runtime/Scene/SceneManager.h**
- `[14]` **Runtime/Serialization/MetaFactory.cpp**
- `[13]` **Editor/Panel/EditorMainBar.cpp**
- `[12]` **Runtime/Managers/Manager.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[12]` **Editor/Panel/EditorInspectorPanel.cpp**
- `[11]` **Runtime/GameObject/MeshFilter.cpp**
- `[11]` **Runtime/Scene/SceneManager.cpp**
- `[10]` **Runtime/Serialization/MetaFactory.h**
- `[9]` **Runtime/Core/PublicStruct.h**
- `[9]` **Runtime/Renderer/Renderer.cpp**
- `[8]` **Editor/Panel/EditorHierarchyPanel.cpp**
- `[8]` **Editor/Panel/EditorInspectorPanel.h**
- `[7]` **Editor/EditorSettings.h**
- `[7]` **Runtime/PreCompiledHeader.h**
- `[7]` **Runtime/Renderer/RenderSorter.h**
- `[7]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[6]` **Runtime/Renderer/RenderCommand.h**
- `[6]` **Runtime/Renderer/RenderContext.h**
- `[6]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[6]` **Assets/Shader/SimpleTestShader.hlsl**
- `[5]` **Runtime/Graphics/GPUSceneManager.cpp**
- `[5]` **Runtime/Graphics/ModelData.cpp**
- `[5]` **Runtime/Renderer/RenderAPI.h**
- `[5]` **Runtime/Renderer/RenderEngine.cpp**
- `[5]` **Runtime/Renderer/Renderer.h**
- `[5]` **Runtime/Settings/ProjectSettings.h**

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
        virtual json SerializedFields() const override {
            return json{
                {"Fov", mFov},
                {"Aspect", mAspect},
                {"Near", mNear},
                {"Far", mFar},
                {"Width", mWidth},
                {"Height", mHeight}
            };
        }
        
        virtual void DeserializedFields(const json& data) override {
            data.at("Fov").get_to(mFov);
            data.at("Aspect").get_to(mAspect);
            data.at("Near").get_to(mNear);
            data.at("Far").get_to(mFar);
            data.at("Width").get_to(mWidth);
            data.at("Height").get_to(mHeight);
        }
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
            transform->AddChild(this);
```
...
```cpp
        }

        inline void AddChild(Transform* transform)
        {
            childTransforms.push_back(transform);
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
        virtual json SerializedFields() { return json::object(); }
        virtual void DeserializedFields(const json& j)  {};
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
{
    using json = nlohmann::json;
    class CameraController : public MonoBehaviour
    {
    public:
        CameraController() = default;
        CameraController(GameObject* go);
        virtual ~CameraController() override {};

        virtual void Update() override;
        virtual const char* GetScriptName() const override;
        virtual json SerializedFields() const override;
        virtual void DeserializedFields(const json& j) override;
    public:
        float testVal = 0;
    };
```

### File: `Runtime/GameObject/Component.h`
```cpp

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