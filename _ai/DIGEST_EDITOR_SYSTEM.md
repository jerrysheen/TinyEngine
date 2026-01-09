# Architecture Digest: EDITOR_SYSTEM
> Auto-generated. Focus: Editor, Panel, Inspector, Hierarchy, Gizmo, GUI, Widget, Console

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 编辑器提供调试与可视化界面，支撑迭代与资产编辑。
- 关注面板/GUI管理与场景交互。

## Key Files Index
- `[78]` **Editor/Panel/EditorHierarchyPanel.h** *(Content Included)*
- `[74]` **Editor/Panel/EditorConsolePanel.h** *(Content Included)*
- `[74]` **Editor/Panel/EditorInspectorPanel.h** *(Content Included)*
- `[55]` **Editor/EditorGUIManager.h** *(Content Included)*
- `[52]` **Editor/D3D12/D3D12EditorGUIManager.h** *(Content Included)*
- `[52]` **Editor/Panel/EditorProjectPanel.h** *(Content Included)*
- `[52]` **Editor/Panel/EditorGameViewPanel.h** *(Content Included)*
- `[48]` **Editor/Panel/EditorPanel.h** *(Content Included)*
- `[45]` **Editor/EditorSettings.h** *(Content Included)*
- `[40]` **Editor/Panel/EditorMainBar.h** *(Content Included)*
- `[12]` **Runtime/Renderer/Renderer.h**
- `[11]` **premake5.lua**
- `[9]` **Runtime/Renderer/RenderCommand.h**
- `[4]` **Runtime/Core/Profiler.h**
- `[4]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[4]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[3]` **Runtime/PreCompiledHeader.h**
- `[3]` **Runtime/Resources/ResourceManager.h**
- `[3]` **Runtime/Renderer/RenderEngine.h**
- `[2]` **Runtime/CoreAssert.h**
- `[2]` **Runtime/EngineCore.h**
- `[2]` **Runtime/Scripts/CameraController.h**
- `[2]` **Runtime/Graphics/MaterialLayout.h**
- `[2]` **Runtime/Graphics/Texture.h**
- `[2]` **Runtime/Graphics/GPUSceneManager.h**
- `[2]` **Runtime/Graphics/ModelData.h**
- `[2]` **Runtime/Graphics/ComputeShader.h**
- `[2]` **Runtime/Graphics/IGPUResource.h**
- `[2]` **Runtime/Graphics/GPUTexture.h**
- `[2]` **Runtime/Graphics/RenderTexture.h**
- `[2]` **Runtime/Graphics/ModelUtils.h**
- `[2]` **Runtime/Graphics/GPUBufferAllocator.h**
- `[2]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[2]` **Runtime/Graphics/Material.h**
- `[2]` **Runtime/Graphics/Shader.h**
- `[2]` **Runtime/Graphics/MaterialInstance.h**
- `[2]` **Runtime/Scene/SceneManager.h**
- `[2]` **Runtime/Scene/Scene.h**
- `[2]` **Runtime/Resources/Resource.h**
- `[2]` **Runtime/Resources/ResourceHandle.h**
- `[2]` **Runtime/Resources/Asset.h**
- `[2]` **Runtime/Serialization/MetaFactory.h**
- `[2]` **Runtime/Serialization/JsonSerializer.h**
- `[2]` **Runtime/Serialization/ComponentFactory.h**
- `[2]` **Runtime/Serialization/BaseTypeSerialization.h**
- `[2]` **Runtime/Serialization/MetaData.h**
- `[2]` **Runtime/Serialization/AssetSerialization.h**
- `[2]` **Runtime/Serialization/MetaLoader.h**
- `[2]` **Runtime/Core/PublicEnum.h**
- `[2]` **Runtime/Core/PublicStruct.h**
- `[2]` **Runtime/Core/InstanceID.h**
- `[2]` **Runtime/Core/Object.h**
- `[2]` **Runtime/Core/Game.h**
- `[2]` **Runtime/Core/Allocator/LinearAllocator.h**
- `[2]` **Runtime/Core/Concurrency/CpuEvent.h**
- `[2]` **Runtime/Managers/Manager.h**
- `[2]` **Runtime/Managers/WindowManager.h**
- `[2]` **Runtime/Platforms/Windows/WindowManagerWindows.h**
- `[2]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[2]` **Runtime/Platforms/D3D12/d3dUtil.h**

## Evidence & Implementation Details

### File: `Editor/Panel/EditorHierarchyPanel.h`
```cpp
#include "imgui.h"

namespace EngineEditor
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

}
```

### File: `Editor/Panel/EditorConsolePanel.h`
```cpp
#include "EditorPanel.h"

namespace EngineEditor
{
    class EditorConsolePanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorConsolePanel() override;
    };

}
```

### File: `Editor/Panel/EditorInspectorPanel.h`
```cpp
#include "GameObject/Transform.h"

namespace EngineEditor
{
    class EditorInspectorPanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorInspectorPanel() override;
        void DrawTransformComponent(EngineCore::Transform* transform);

        // 一个小工具：把角度差wrap到[-180,180]，避免359→-1的大跳
        static inline float WrapDelta180(float d) {
            while (d > 180.f) d -= 360.f;
            while (d < -180.f) d += 360.f;
            return d;
        }

        // 给每个Transform维护一份UI角度缓存（仅用于显示/差分计算）
        struct EulerUICache 
        { 
            EulerUICache() = default;
            Vector3 eulerDeg = { 0,0,0 }; 
            bool seeded = false; 
        };

        static std::unordered_map<EngineCore::Transform*, EulerUICache> sEulerCache;
    };

}
```

### File: `Editor/EditorGUIManager.h`
```cpp
#include "GameObject/GameObject.h"

namespace EngineEditor
{
    using GameObject = EngineCore::GameObject;
    class EditorPanel;
    class EditorGUIManager
    {
    public:
    // 对象不允许纯虚函数，所以要用指针，因为是指针，所以*sInstance表示对象，外面返回引用&就行。
        static EngineEditor::EditorGUIManager* s_Instance;
        static EngineEditor::EditorGUIManager* GetInstance(){ return (s_Instance);};
        virtual ~EditorGUIManager();
        
        static void Create();
        static void OnDestory();
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        // Update不需要实现， 因为是一个纯Payload，无任何信息/
        void Update();
        // 录入指令部分，不需要有具体RenderAPI实现。
        void Render();
        void InitPanel();

        inline void SetCurrentSelected(GameObject* go) 
        { 
            currentSelected = go; 
        };
        inline GameObject* GetCurrentSelected()
        { 
            return currentSelected;
        };
    private:
        std::vector<EditorPanel*> mPanelList;
        GameObject* currentSelected = nullptr;
    };


}
```

### File: `Editor/D3D12/D3D12EditorGUIManager.h`
```cpp
#include "imgui.h"

namespace EngineEditor
{
    class D3D12EditorGUIManager : public EditorGUIManager
    {
    public:
        // 渲染线程beginFrame，重置CommandList
        virtual void BeginFrame() override;
        // 渲染线程EndFrame，提交指令
        virtual void EndFrame() override;
        static void ApplyDpiScale(float scale)
        {
            // 1) 缩放 ImGui Style
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(scale);

            // 2) 重新构建字体（把像素大小乘上 scale）
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->Clear();
            // 举例：原 16px 字体，按比例放大
            io.Fonts->AddFontFromFileTTF("c:/Windows/Fonts/segoeui.ttf", 16.0f * scale);
            io.Fonts->Build();

            // 3) 如果你不想重建字体，也可以简单用全局缩放(体验略差)
            // io.FontGlobalScale = scale; // 简易方案（字体不会变清晰度）
        }
        D3D12EditorGUIManager();
        ~D3D12EditorGUIManager();
	private:

		const uint32_t maxDescriptorNum = 256;

		UINT descriptorSize;
		std::vector<bool> descriptorUseState;
		ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
		ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
		std::vector<ComPtr<ID3D12CommandAllocator>> commandAllocators;

		void InitForDirectX12();
		UINT GetNextAvailablePos();

    };
}
```

### File: `Editor/Panel/EditorProjectPanel.h`
```cpp
#include "EditorPanel.h"

namespace EngineEditor
{
    class EditorProjectPanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorProjectPanel() override;
    };

}
```

### File: `Editor/Panel/EditorGameViewPanel.h`
```cpp
#include "EditorPanel.h"

namespace EngineEditor
{
    class EditorGameViewPanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorGameViewPanel() override;
    };

}
```

### File: `Editor/Panel/EditorPanel.h`
```cpp
#pragma once

namespace EngineEditor
{
    class EditorPanel
    {
    public: 
        virtual void DrawGUI() = 0;
        virtual ~EditorPanel() = 0;
    };

}
```

### File: `Editor/EditorSettings.h`
```cpp


namespace EngineEditor
{
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


}
```

### File: `Editor/Panel/EditorMainBar.h`
```cpp


namespace EngineEditor
{
    using ResourceManager = EngineCore::ResourceManager;
    using AssetID = EngineCore::AssetID;
    using Resource = EngineCore::Resource;
    using AssetType = EngineCore::AssetType;
    using Material = EngineCore::Material;
    using Shader = EngineCore::Shader;
    using ModelData = EngineCore::ModelData;
    using Texture = EngineCore::Texture;
    //using MaterialMetaData = EngineCore::MaterialMetaData;
    using TextureMetaData = EngineCore::TextureMetaData;
    using MetaData = EngineCore::MetaData;
    using SceneManager = EngineCore::SceneManager;
    using Scene = EngineCore::Scene;

    class EditorMainBar : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorMainBar() override;
    private:
        void GenerateSceneMetaFile();
        void CreateHouseGameObject();

    };

}
```