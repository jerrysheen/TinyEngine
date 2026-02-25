# Architecture Digest: ENGINE_OVERVIEW
> Auto-generated. Focus: Runtime/Entry.cpp, Runtime/Core/Game.h, Runtime/Settings, EngineCore, Game, RenderEngine, SceneManager, Settings, ProjectSettings, RenderSettings, Runtime/Utils

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 引擎入口、主循环与模块初始化/销毁流程。
- 定位Game/Render/Scene之间的耦合与调度关系。

## Key Files Index
- `[63]` **Runtime/Settings/ProjectSettings.cpp** *(Content Included)*
- `[59]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[59]` **Runtime/Settings/ProjectSettings.h** *(Content Included)*
- `[44]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[41]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[38]` **Runtime/Core/Game.h** *(Content Included)*
- `[37]` **Runtime/Entry.cpp** *(Content Included)*
- `[35]` **Editor/Panel/EditorGameViewPanel.cpp** *(Content Included)*
- `[33]` **Runtime/Scene/SceneManager.h** *(Content Included)*
- `[30]` **Editor/EditorSettings.cpp** *(Content Included)*
- `[29]` **Editor/EditorSettings.h** *(Content Included)*
- `[29]` **Runtime/GameObject/GameObject.cpp** *(Content Included)*
- `[28]` **Runtime/GameObject/GameObject.h** *(Content Included)*
- `[28]` **Runtime/Renderer/RenderEngine.h** *(Content Included)*
- `[24]` **Runtime/EngineCore.h** *(Content Included)*
- `[24]` **Editor/Panel/EditorGameViewPanel.h** *(Content Included)*
- `[23]` **Runtime/GameObject/Camera.cpp** *(Content Included)*
- `[23]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[19]` **Runtime/GameObject/Transform.h** *(Content Included)*
- `[17]` **Runtime/GameObject/MeshRenderer.cpp** *(Content Included)*
- `[16]` **Runtime/GameObject/Component.h**
- `[16]` **Runtime/GameObject/ComponentType.h**
- `[16]` **Runtime/GameObject/MeshFilter.cpp**
- `[16]` **Runtime/GameObject/MeshRenderer.h**
- `[16]` **Runtime/GameObject/Transform.cpp**
- `[16]` **Editor/Panel/EditorMainBar.cpp**
- `[15]` **Runtime/GameObject/MeshFilter.h**
- `[15]` **Runtime/GameObject/MonoBehaviour.h**
- `[14]` **Runtime/GameObject/Camera.h**
- `[14]` **Runtime/GameObject/MonoBehaviour.cpp**
- `[14]` **Runtime/Serialization/SceneLoader.h**
- `[13]` **Runtime/Utils/HashCombine.h**
- `[13]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.cpp**
- `[13]` **Editor/Panel/EditorHierarchyPanel.cpp**
- `[12]` **Runtime/GameObject/Component.cpp**
- `[12]` **Runtime/Renderer/RenderContext.cpp**
- `[12]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[11]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[11]` **Editor/Panel/EditorMainBar.h**
- `[10]` **Runtime/Serialization/MaterialLoader.h**
- `[10]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp**
- `[10]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[9]` **Runtime/Renderer/Culling.cpp**
- `[9]` **Runtime/Renderer/RenderContext.h**
- `[9]` **Runtime/Scene/Scene.h**
- `[9]` **Runtime/Serialization/DDSTextureLoader.h**
- `[9]` **Runtime/Platforms/D3D12/d3dUtil.cpp**
- `[9]` **Runtime/Platforms/Windows/WindowManagerWindows.cpp**
- `[9]` **Editor/Panel/EditorHierarchyPanel.h**
- `[8]` **Editor/EditorGUIManager.h**
- `[8]` **Runtime/Renderer/BatchManager.cpp**
- `[8]` **Runtime/Scene/BistroSceneLoader.h**
- `[8]` **Runtime/Serialization/MeshLoader.h**
- `[8]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[8]` **Editor/Panel/EditorConsolePanel.cpp**
- `[8]` **Editor/Panel/EditorInspectorPanel.cpp**
- `[7]` **Runtime/Renderer/Renderer.h**
- `[7]` **Runtime/Scene/Scene.cpp**
- `[7]` **Runtime/Serialization/ShaderLoader.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**

## Evidence & Implementation Details

### File: `Runtime/Settings/ProjectSettings.cpp`
```cpp
#include "ProjectSettings.h"

namespace EngineCore
{
    bool ProjectSettings::s_Initialized = false;
    void ProjectSettings::Initialize()
    {
        PathSettings::Initialize();
        s_Initialized = true;
    }

    std::string PathSettings::s_ProjectRoot = "";
    std::string PathSettings::s_AssetsPath = "";
    void PathSettings::Initialize()
    {
        #ifdef ENGINE_ROOT_PATH
            // 开发模式：使用编译时注入的绝对路径
            s_ProjectRoot = ENGINE_ROOT_PATH;
            s_AssetsPath = ASSETS_PATH;
        #else
            // 发布模式：基于可执行文件位置
            s_ProjectRoot = GetExecutablePath() + "";
            s_AssetsPath = s_ProjectRoot + "/Assets";
        #endif
    }

    std::string PathSettings::GetExecutablePath()
    {
        char buffer[MAX_PATH];
        DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
        
        // 错误处理：检查是否成功获取路径
        if (length == 0) 
        {
            // 获取失败，返回当前工作目录作为备选
            #ifdef DEBUG_MODE
                printf("[Error] Failed to get executable path, using current directory\n");
            #endif
            return ".";
        }
        
        // 检查路径是否被截断（buffer太小）
        if (length == MAX_PATH) 
        {
            #ifdef DEBUG_MODE
                printf("[Warning] Executable path may be truncated\n");
            #endif
        }
        
        std::string fullPath(buffer, length);
        
        // 查找最后一个斜杠或反斜杠的位置
        size_t lastSlash = fullPath.find_last_of("\\/");
        
        if (lastSlash != std::string::npos) 
        {
            // 返回目录部分（不包含exe文件名）
            return fullPath.substr(0, lastSlash);
        }
        
        // 如果没有找到路径分隔符（不太可能），返回当前目录
        return ".";
    }

    std::string PathSettings::ResolveAssetPath(const std::string& relativePath)
    {
        // 如果已经是绝对路径，直接返回
        if (relativePath.size() >= 2 && relativePath[1] == ':') 
            return relativePath;
        
        // 拼接Assets路径
        std::string fullPath = s_AssetsPath + relativePath;
        
        // 标准化路径
        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
        
        return fullPath;
    }


```

### File: `Runtime/Core/Game.cpp`
```cpp
#include "Settings/ProjectSettings.h"
#include "Resources/AssetRegistry.h"
namespace EngineCore
{
    void Game::Launch()
    {
        ProjectSettings::Initialize();
        // InitManagers Here.
        RenderEngine::Create();
        ResourceManager::Create();
        SceneManager::Create();
        JobSystem::Create();
        AssetRegistry::Create();
        ASSERT(!(RenderSettings::s_EnableVertexPulling == true && RenderSettings::s_RenderPath == RenderSettings::RenderPathType::Legacy));
        //std::cout << "Launch Game" << std::endl;
        // init Manager...
        #ifdef EDITOR
        EngineEditor::EditorGUIManager::Create();
        #endif
        while(!WindowManager::GetInstance()->WindowShouldClose())
        {
            mFrameIndex++;
            PROFILER_FRAME_MARK("TinyProfiler");
            Update(mFrameIndex);

            Render();

            EndFrame();
        }

        // 明确的关闭流程（顺序很重要！）
        Shutdown();
    }

    void Game::Shutdown()
    {
        std::cout << "Game shutting down..." << std::endl;

        // 1. 先停止渲染线程（最重要！）
        //    必须在销毁任何渲染资源之前停止
        RenderEngine::Destory();
        std::cout << "RenderEngine destroyed." << std::endl;

        // 2. 销毁编辑器UI
        #ifdef EDITOR
        EngineEditor::EditorGUIManager::OnDestory();
        std::cout << "EditorGUIManager destroyed." << std::endl;
        #endif

        // 3. 销毁场景（包含所有GameObject）
        SceneManager::Destroy();
        std::cout << "SceneManager destroyed." << std::endl;

        // 4. 最后销毁资源管理器
        ResourceManager::GetInstance()->Destroy();
        std::cout << "ResourceManager destroyed." << std::endl;

        std::cout << "Game shutdown complete." << std::endl;
    }

    void Game::Update(uint32_t frameIndex)
    {
        PROFILER_ZONE("MainThread::GameUpdate");
        ResourceManager::GetInstance()->Update();
        SceneManager::GetInstance()->Update(frameIndex);
        RenderEngine::GetInstance()->Update(frameIndex);
    }

    void Game::Render()
    {
        PROFILER_ZONE("MainThread::RenderTick");
        RenderEngine::GetInstance()->Tick();
    }

    void Game::EndFrame()
    {
        SceneManager::GetInstance()->EndFrame();
        RenderEngine::GetInstance()->EndFrame();
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


    class RenderSettings
    {
    public:
        enum class RenderPathType
        {
            Legacy,
            GPUScene
        };
        static RenderPathType s_RenderPath;
        static bool s_EnableVertexPulling;
    };

    class ProjectSettings
    {
    public:
        static void Initialize();
        static bool s_Initialized;
    };

}
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
        void Update(uint32_t frameIndex);
        void Render();
        void EndFrame();
        void Shutdown();

        uint32_t mFrameIndex = 0;
    };

}
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
        
        static void UpdateLayout(float windowWidth, float windowHeight);

    };
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
        inline Scene* GetOwnerScene() { return ownerScene; }
        inline void SetOwnerScene(Scene* scene) { ownerScene = scene; }
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
        components.try_emplace(type, component);
        if(type == ComponentType::Script)
        {
            scripts.push_back(reinterpret_cast<MonoBehaviour*>(component));
        }
        return component;
    }



}
```

### File: `Runtime/Renderer/RenderEngine.h`
```cpp
#include "Scene/CPUScene.h"

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
        void Update(uint32_t frameID);
        static void Create();
        void Tick();
        void EndFrame();
        
        static void OnResize(int width, int height);
        static void OnDrawGUI();
        static void Destory();
        RenderEngine(){};
        ~RenderEngine(){};
        static void WaitForLastFrameFinished();
        static void SignalMainThreadSubmited();
        inline CPUScene& GetCPUScene(){return mCPUScene;}
        inline GPUScene& GetGPUScene(){return mGPUScene;}
        static GPUSceneRenderPath gpuSceneRenderPath;
        static LagacyRenderPath lagacyRenderPath;

    private:
        static std::unique_ptr<RenderEngine> s_Instance;
        static RenderContext renderContext;

        GPUScene mGPUScene;
        CPUScene mCPUScene;

        void ComsumeDirtySceneRenderNode();
        void ComsumeDirtyCPUSceneRenderNode();
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
#include "Scene/SceneManager.h"

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
```