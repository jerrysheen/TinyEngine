# Architecture Digest: ENGINE_OVERVIEW
> Auto-generated. Focus: Runtime/Entry, EngineCore, Game, RenderEngine, SceneManager, Settings, Application, Runtime/Utils

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 引擎入口、主循环与模块初始化/销毁流程。
- 定位Game/Render/Scene之间的耦合与调度关系。

## Key Files Index
- `[33]` **Runtime/Scene/SceneManager.h** *(Content Included)*
- `[29]` **Editor/EditorSettings.h** *(Content Included)*
- `[28]` **Runtime/Core/Game.h** *(Content Included)*
- `[28]` **Runtime/GameObject/GameObject.h** *(Content Included)*
- `[28]` **Runtime/Renderer/RenderEngine.h** *(Content Included)*
- `[27]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[27]` **Runtime/Settings/ProjectSettings.h** *(Content Included)*
- `[24]` **Runtime/EngineCore.h** *(Content Included)*
- `[24]` **Editor/Panel/EditorGameViewPanel.h** *(Content Included)*
- `[19]` **Runtime/GameObject/Transform.h** *(Content Included)*
- `[16]` **Runtime/GameObject/ComponentType.h**
- `[16]` **Runtime/GameObject/Component.h**
- `[16]` **Runtime/GameObject/MeshRenderer.h**
- `[15]` **Runtime/GameObject/MonoBehaviour.h**
- `[15]` **Runtime/GameObject/MeshFilter.h**
- `[14]` **Runtime/GameObject/Camera.h**
- `[13]` **Runtime/Utils/HashCombine.h**
- `[11]` **Runtime/Serialization/ComponentFactory.h**
- `[11]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[11]` **Editor/Panel/EditorMainBar.h**
- `[9]` **Runtime/Scene/Scene.h**
- `[9]` **Runtime/Serialization/MetaFactory.h**
- `[9]` **Runtime/Renderer/RenderContext.h**
- `[9]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[9]` **Editor/Panel/EditorHierarchyPanel.h**
- `[8]` **Runtime/Serialization/JsonSerializer.h**
- `[8]` **Editor/EditorGUIManager.h**
- `[7]` **Runtime/Serialization/BaseTypeSerialization.h**
- `[7]` **Runtime/Serialization/AssetSerialization.h**
- `[6]` **Runtime/Resources/ResourceManager.h**
- `[5]` **Runtime/Scripts/CameraController.h**
- `[5]` **Runtime/Resources/Asset.h**
- `[5]` **Runtime/Core/PublicStruct.h**
- `[5]` **Runtime/Core/Profiler.h**
- `[5]` **Runtime/Math/Math.h**
- `[5]` **Runtime/Renderer/BatchManager.h**
- `[5]` **Runtime/Renderer/Culling.h**
- `[5]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[5]` **Editor/Panel/EditorInspectorPanel.h**
- `[4]` **Runtime/Resources/ResourceHandle.h**
- `[4]` **Runtime/Core/InstanceID.h**
- `[4]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[4]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[4]` **Runtime/Renderer/RenderSorter.h**
- `[4]` **Runtime/Renderer/RenderAPI.h**
- `[4]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h**
- `[3]` **premake5.lua**
- `[3]` **Runtime/Graphics/MaterialLayout.h**
- `[3]` **Runtime/Graphics/Texture.h**
- `[3]` **Runtime/Graphics/ModelData.h**
- `[3]` **Runtime/Graphics/ComputeShader.h**
- `[3]` **Runtime/Graphics/IGPUResource.h**
- `[3]` **Runtime/Graphics/GPUTexture.h**
- `[3]` **Runtime/Graphics/RenderTexture.h**
- `[3]` **Runtime/Graphics/ModelUtils.h**
- `[3]` **Runtime/Graphics/GPUBufferAllocator.h**
- `[3]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[3]` **Runtime/Graphics/Material.h**
- `[3]` **Runtime/Graphics/Shader.h**
- `[3]` **Runtime/Graphics/MaterialInstance.h**

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

### File: `Editor/EditorSettings.h`
```cpp
using Vector2 = EngineCore::Vector2;
    // Editor
    class EditorSettings
    {
    private:
        // StartPos 和 Size都是0~1的ratio
        static Vector2  hierarchyStartPos;
        static Vector2  hierarchySize;
        static Vector2  consoleStartPos;
        static Vector2  consoleSize;
        static Vector2  projectStartPos;
        static Vector2  projectSize;
        static Vector2  inspectorStartPos;
        static Vector2  inspectorSize;
        static Vector2  mainBarStartPos;
        static Vector2  mainBarSize;
        static Vector2  gameViewStartPos;
        static Vector2  gameViewSize;

        static Vector2 currentWindowSize;
    public:
        inline static Vector2 GetHierarchyPanelStartPos(){return hierarchyStartPos * currentWindowSize;};
        inline static Vector2 GetHierarchyPanelEndPos(){return (hierarchyStartPos + hierarchySize) * currentWindowSize; };
        inline static Vector2 GetHierarchyPanelSize(){return hierarchySize * currentWindowSize;};

        inline static Vector2 GetConsolePanelStartPos(){return consoleStartPos * currentWindowSize;};
        inline static Vector2 GetConsolePanelEndPos(){return (consoleStartPos + consoleSize) * currentWindowSize;};
        inline static Vector2 GetConsolePanelSize(){return consoleSize * currentWindowSize;};

        inline static Vector2 GetProjectPanelStartPos(){return projectStartPos * currentWindowSize;};
        inline static Vector2 GetProjectPanelEndPos(){return (projectStartPos + projectSize) * currentWindowSize;};
        inline static Vector2 GetProjectPanelSize(){return projectSize * currentWindowSize;};

        inline static Vector2 GetInspectorPanelStartPos(){return inspectorStartPos * currentWindowSize;};
        inline static Vector2 GetInspectorPanelEndPos(){return (inspectorStartPos + inspectorSize) * currentWindowSize;};
        inline static Vector2 GetInspectorPanelSize(){return inspectorSize * currentWindowSize;};

        inline static Vector2 GetMainBarPanelStartPos(){return mainBarStartPos * currentWindowSize;};
        inline static Vector2 GetMainBarPanelEndPos(){return (mainBarStartPos + mainBarSize) * currentWindowSize;};
        inline static Vector2 GetMainBarPanelSize(){return mainBarSize * currentWindowSize;};

        inline static Vector2 GetGameViewPanelStartPos(){return gameViewStartPos * currentWindowSize;};
        inline static Vector2 GetGameViewPanelEndPos(){return (gameViewStartPos + gameViewSize) * currentWindowSize;};
        inline static Vector2 GetGameViewPanelSize(){return gameViewSize * currentWindowSize;};
        
        static void UpdateLayout(){};

    };
```

### File: `Runtime/Core/Game.h`
```cpp


namespace EngineCore
{
    class Game
    {
    public:
        static std::unique_ptr<Game> m_Instance;
        // 回传的是一个对象的引用，所以返回*ptr
        static Game* GetInstance()
        {
            if(m_Instance == nullptr)
            {
                m_Instance = std::make_unique<Game>();
            }
            return m_Instance.get();
        };
        Game(){};
        ~Game(){};

        void Launch();
    private:
        void Update();
        void Render();
        void EndFrame();
        void Shutdown();
    };

}
```

### File: `Runtime/GameObject/GameObject.h`
```cpp
#include "MonoBehaviour.h"

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

    template<typename T>
    inline T* GameObject::GetComponent() const
    {
        // const 后不能直接用 conponents[type]， 因为可能会产生修改。
        ComponentType type = T::GetStaticType();
        auto it = components.find(type);
        if(it != components.end())
        {
            return static_cast<T*>(it->second);            
        }
        return nullptr;
    }

    template<typename T>
    inline T* GameObject::AddComponent()
    {
        // todo: 这边确实该用multimap的， 因为原则上MonoBehaviour可以挂多个

        ComponentType type = T::GetStaticType();
        if(components.count(type) > 0)
        {
            return nullptr;
        }
        T* component = new T(this);
```

### File: `Runtime/Renderer/RenderEngine.h`
```cpp
#include "Renderer/RenderPath/GPUSceneRenderPath.h"

namespace EngineCore
{
    // 平台无关类， 负责拉起循环而已，内部各种渲染类组成。
    // 首先明确下这些函数都在负责什么功能。
    // begin render: RenderAPI->BeginFrame() 等待上一帧结束，OnResize处理
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
    
}
```

### File: `Runtime/Graphics/GPUSceneManager.h`
```cpp
#include "Resources/ResourceHandle.h"

namespace EngineCore
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

}
```

### File: `Runtime/Settings/ProjectSettings.h`
```cpp
#include <iostream>

namespace EngineCore
{
    class PathSettings
    {
    public:
        static void Initialize();
        static bool sInitialized;
        // 获取各种路径
        static std::string GetProjectRoot(){ return s_ProjectRoot; };
        static std::string GetAssetsPath(){ return s_AssetsPath; };
        static std::string ResolveAssetPath(const std::string& relativePath);
        
        // 便捷方法：解析资源相对路径为绝对路径
        //static std::string ResolveAssetPath(const std::string& relativePath);
        
        // 获取可执行文件路径（用于发布模式）
        static std::string GetExecutablePath();
    private:
        static std::string s_ProjectRoot;
        static std::string s_AssetsPath;        
    };

    class WindowSettings
    {
    public:
        static int s_WindowWidth;
        static int s_WindowHeight;
    };


    class RenderSettings{};

    class ProjectSettings
    {
    public:
        static void Initialize();
        static bool s_Initialized;
    };

}
```

### File: `Editor/Panel/EditorGameViewPanel.h`
```cpp
namespace EngineEditor
{
    class EditorGameViewPanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorGameViewPanel() override;
    };
```

### File: `Runtime/GameObject/Transform.h`
```cpp
#include "Serialization/BaseTypeSerialization.h"

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
```