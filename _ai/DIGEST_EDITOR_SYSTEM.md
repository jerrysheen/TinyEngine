# Architecture Digest: EDITOR_SYSTEM
> Auto-generated. Focus: Editor, Editor/Panel, Editor/D3D12, Editor/EditorSerialization, Editor/Mac, Panel, Inspector, Hierarchy, Gizmo, GUI, Widget, Console

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 编辑器提供调试与可视化界面，支撑迭代与资产编辑。
- 关注面板/GUI管理与场景交互。

## Key Files Index
- `[85]` **Editor/Panel/EditorConsolePanel.cpp** *(Content Included)*
- `[85]` **Editor/Panel/EditorHierarchyPanel.cpp** *(Content Included)*
- `[85]` **Editor/Panel/EditorInspectorPanel.cpp** *(Content Included)*
- `[83]` **Editor/Panel/EditorConsolePanel.h** *(Content Included)*
- `[83]` **Editor/Panel/EditorHierarchyPanel.h** *(Content Included)*
- `[83]` **Editor/Panel/EditorInspectorPanel.h** *(Content Included)*
- `[62]` **Editor/EditorGUIManager.cpp** *(Content Included)*
- `[61]` **Editor/Panel/EditorGameViewPanel.h** *(Content Included)*
- `[61]` **Editor/Panel/EditorProjectPanel.h** *(Content Included)*
- `[60]` **Editor/Panel/EditorGameViewPanel.cpp** *(Content Included)*
- `[60]` **Editor/Panel/EditorProjectPanel.cpp** *(Content Included)*
- `[57]` **Editor/D3D12/D3D12EditorGUIManager.h** *(Content Included)*
- `[57]` **Editor/Panel/EditorPanel.cpp** *(Content Included)*
- `[57]` **Editor/Panel/EditorPanel.h** *(Content Included)*
- `[55]` **Editor/D3D12/D3D12EditorGUIManager.cpp** *(Content Included)*
- `[51]` **Editor/EditorGUIManager.h** *(Content Included)*
- `[49]` **Editor/Panel/EditorMainBar.cpp** *(Content Included)*
- `[49]` **Editor/Panel/EditorMainBar.h** *(Content Included)*
- `[45]` **Editor/EditorSettings.h** *(Content Included)*
- `[40]` **Editor/EditorSettings.cpp** *(Content Included)*
- `[20]` **Runtime/Entry.cpp**
- `[17]` **Runtime/Core/Game.cpp**
- `[12]` **Runtime/Renderer/RenderEngine.cpp**
- `[11]` **Runtime/Renderer/Renderer.h**
- `[8]` **Runtime/Graphics/GPUSceneManager.cpp**
- `[8]` **Runtime/Scene/SceneManager.cpp**
- `[7]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[6]` **premake5.lua**
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[5]` **Runtime/Renderer/RenderCommand.h**
- `[4]` **Runtime/Core/Profiler.h**
- `[4]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[4]` **Runtime/Platforms/D3D12/d3dUtil.cpp**
- `[4]` **Assets/Shader/BlitShader.hlsl**
- `[4]` **Assets/Shader/GPUCulling.hlsl**
- `[4]` **Assets/Shader/SimpleTestShader.hlsl**
- `[4]` **Assets/Shader/StandardPBR.hlsl**
- `[4]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[3]` **Runtime/PreCompiledHeader.h**
- `[3]` **Runtime/Resources/ResourceManager.h**
- `[3]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[3]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[2]` **Runtime/CoreAssert.h**
- `[2]` **Runtime/EngineCore.h**
- `[2]` **Runtime/Core/Game.h**
- `[2]` **Runtime/Core/InstanceID.h**
- `[2]` **Runtime/Core/Object.h**
- `[2]` **Runtime/Core/PublicEnum.h**
- `[2]` **Runtime/Core/PublicStruct.h**
- `[2]` **Runtime/Core/ThreadSafeQueue.h**
- `[2]` **Runtime/GameObject/Camera.h**
- `[2]` **Runtime/GameObject/Component.h**
- `[2]` **Runtime/GameObject/ComponentType.h**
- `[2]` **Runtime/GameObject/GameObject.h**
- `[2]` **Runtime/GameObject/MeshFilter.h**
- `[2]` **Runtime/GameObject/MeshRenderer.h**
- `[2]` **Runtime/GameObject/MonoBehaviour.h**
- `[2]` **Runtime/GameObject/Transform.h**
- `[2]` **Runtime/Graphics/ComputeShader.h**

## Evidence & Implementation Details

### File: `Editor/Panel/EditorConsolePanel.cpp`
```cpp
#include "Core/Profiler.h"
#include <cstring> // 顶部加一下
namespace EngineEditor
{
    // Console 透明度设置为0.0， 只用来接受以及处理一些SceneView事件
    void EditorConsolePanel::DrawGUI()
    {
        ImGui::SetNextWindowPos(ImVec2((float)EditorSettings::GetConsolePanelStartPos().x, (float)EditorSettings::GetConsolePanelStartPos().y));
        ImGui::SetNextWindowSize(ImVec2((float)EditorSettings::GetConsolePanelSize().x, (float)EditorSettings::GetConsolePanelSize().y));

        if (ImGui::Begin("Console - Profiler", NULL,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize))
        {
#if ENABLE_PROFILER
            // 获取上一帧的性能数据（当前帧可能还在写入）
            EngineCore::Profiler& profiler = EngineCore::Profiler::Get();
            uint32_t currentFrameIndex = profiler.GetCurrentFrameIndex();
            // 获取前一帧的索引
            uint32_t prevFrameIndex = (currentFrameIndex > 0) ? (currentFrameIndex - 1) : (299); // kMaxFrames - 1
            const EngineCore::ProfilerFrame& frame = profiler.GetFrame(prevFrameIndex);

            // 显示帧信息
            float frameTimeMs = profiler.GetLastFrameTimeMs();
            float fps = profiler.GetFPS();
            ImGui::Text("Frame: %u | Frame Time: %.2f ms | FPS: %.1f", prevFrameIndex, frameTimeMs, fps);
            ImGui::Separator();

            // 创建一个可滚动的区域来显示所有事件
            ImGui::BeginChild("ProfilerEvents", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            // 表头
            ImGui::Columns(4, "ProfilerColumns");
            ImGui::SetColumnWidth(0, 300.0f);
            ImGui::SetColumnWidth(1, 100.0f);
            ImGui::SetColumnWidth(2, 100.0f);
            ImGui::SetColumnWidth(3, 80.0f);

            ImGui::Text("Event Name"); ImGui::NextColumn();
            ImGui::Text("Duration (ms)"); ImGui::NextColumn();
            ImGui::Text("Thread ID"); ImGui::NextColumn();
            ImGui::Text("Depth"); ImGui::NextColumn();
            ImGui::Separator();

            // 遍历所有打点事件
            for (const auto& event : frame.events)
            {
                if (event.name != nullptr)
                {
                    float duration = event.endMs - event.startMs;

                    // 根据耗时着色
                    if (duration > 10.0f)
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // 红色：慢
                    else
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // 绿色：快

                    // 根据 depth 添加缩进
                    std::string indentedName = std::string(event.depth * 2, ' ') + event.name;
                    ImGui::Text("%s", indentedName.c_str()); ImGui::NextColumn();
                    ImGui::Text("%.3f", duration); ImGui::NextColumn();
                    ImGui::Text("%u", event.threadID); ImGui::NextColumn();
                    ImGui::Text("%u", event.depth); ImGui::NextColumn();

                    ImGui::PopStyleColor();
                }
            }

     
            ImGui::Columns(1);
            ImGui::EndChild();
#else
            ImGui::Text("Profiler is disabled. Enable ENABLE_PROFILER to see performance data.");
#endif
            ImGui::End();
        }
    }
    
    EditorConsolePanel::~EditorConsolePanel()
    {
```

### File: `Editor/Panel/EditorHierarchyPanel.cpp`
```cpp
#include "EditorGUIManager.h"

namespace EngineEditor
{
    void EditorHierarchyPanel::DrawGUI()
    {
        ImGui::SetNextWindowPos(ImVec2((float)EditorSettings::GetHierarchyPanelStartPos().x, (float)EditorSettings::GetHierarchyPanelStartPos().y));
        ImGui::SetNextWindowSize(ImVec2((float)EditorSettings::GetHierarchyPanelSize().x, (float)EditorSettings::GetHierarchyPanelSize().y));
        
		// ��������������
		if (ImGui::Begin("Hierarchy", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
            nodeIdx = 0;
			auto* scene = EngineCore::SceneManager::GetInstance()->GetCurrentScene();
            if (scene == nullptr ) 
            {
                ImGui::End();
                return;
            }
			for (auto gameObject : scene->rootObjList)
            {
                if (gameObject == nullptr) continue;
				DrawNode(gameObject);
            }
		}
        ImGui::End();

    }

    EditorHierarchyPanel::~EditorHierarchyPanel()
    {
    }


    void EditorHierarchyPanel::DrawNode(GameObject *gameObject)
    {
        nodeIdx++;

        ImGuiTreeNodeFlags nodeFlags = baseFlags;
        if(selectedGO == gameObject)
        {
            nodeFlags |= ImGuiTreeNodeFlags_Selected;
        }

        if(gameObject->transform->childTransforms.size() == 0)
        {
            nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			if (!gameObject->enabled)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(160, 160, 160, 255));
            } 

			ImGui::TreeNodeEx((void*)(intptr_t)nodeIdx, nodeFlags, gameObject->name.c_str());

			if (!gameObject->enabled) 
            {
                ImGui::PopStyleColor();
            }

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				selectedGO = gameObject;
				EditorGUIManager::GetInstance()->SetCurrentSelected(gameObject);
			}

        }
        else
        {
			if (!gameObject->enabled)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(160, 160, 160, 255));
            } 

			bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)nodeIdx, nodeFlags, gameObject->name.c_str());
			
            if (!gameObject->enabled) 
            {
                ImGui::PopStyleColor();
            }
```
...
```cpp
			{
				selectedGO = gameObject;
				EditorGUIManager::GetInstance()->SetCurrentSelected(gameObject);
			}

			if (nodeOpen)
			{
```

### File: `Editor/Panel/EditorInspectorPanel.cpp`
```cpp
#include "EditorGUIManager.h"

namespace EngineEditor
{
    std::unordered_map<EngineCore::Transform*, EditorInspectorPanel::EulerUICache>
        EditorInspectorPanel::sEulerCache;

    using Transform = EngineCore::Transform;
    // Inspector 透明度设置为0.0， 只用来接受以及处理一些SceneView事件
    void EditorInspectorPanel::DrawGUI()
    {
        ImGui::SetNextWindowPos(ImVec2((float)EditorSettings::GetInspectorPanelStartPos().x, (float)EditorSettings::GetInspectorPanelStartPos().y));
        ImGui::SetNextWindowSize(ImVec2((float)EditorSettings::GetInspectorPanelSize().x, (float)EditorSettings::GetInspectorPanelSize().y));
        
        if (ImGui::Begin("Inspector", NULL, 
            ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoResize))
        {
            auto* currentSelected = EditorGUIManager::GetInstance()->GetCurrentSelected();
            if(currentSelected != nullptr)
            {
                auto* transform = currentSelected->GetComponent<Transform>();
                DrawTransformComponent(transform);
            }

        }
        ImGui::End();
    }
    
    void EditorInspectorPanel::DrawTransformComponent(EngineCore::Transform* transform)
    {
        if (transform == nullptr) return;
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (!ImGui::CollapsingHeader("Transform"))
            return;

        ImGui::PushItemWidth(60);

        // --- Position（保持你原来的逻辑：这是局部位移；若你想“世界位移”，可改成TranslateWorld）---
        Vector3 position = transform->GetLocalPosition();
        ImGui::Text("Position    ");
        ImGui::SameLine(); ImGui::Text("X");
        ImGui::SameLine(); bool posXChanged = ImGui::DragFloat("##posX", &position.x, 0.15f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Y");
        ImGui::SameLine(); bool posYChanged = ImGui::DragFloat("##posY", &position.y, 0.15f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Z");
        ImGui::SameLine(); bool posZChanged = ImGui::DragFloat("##posZ", &position.z, 0.15f, -FLT_MAX, FLT_MAX);
        if (posXChanged || posYChanged || posZChanged) {
            transform->SetLocalPosition(position);
        }

        // --- Rotation（改为“增量四元数 + 世界轴左乘”，UI 用缓存）---
        // 1) 取/初始化缓存（仅第一次对这个Transform显示时，从真实姿态做一次seed）
        auto& cache = sEulerCache[transform];
        if (!cache.seeded) {
            cache.eulerDeg = transform->GetLocalEulerAngles(); // 仅一次；之后不再反解
            cache.seeded = true;
        }

        Vector3 uiEuler = cache.eulerDeg; // 用缓存作为当前显示值
        ImGui::Text("Rotation    ");
        ImGui::SameLine(); ImGui::Text("X");
        ImGui::SameLine(); bool rotXChanged = ImGui::DragFloat("##rotX", &uiEuler.x, 0.25f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Y");
        ImGui::SameLine(); bool rotYChanged = ImGui::DragFloat("##rotY", &uiEuler.y, 0.25f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Z");
        ImGui::SameLine(); bool rotZChanged = ImGui::DragFloat("##rotZ", &uiEuler.z, 0.25f, -FLT_MAX, FLT_MAX);

        if (rotXChanged || rotYChanged || rotZChanged) {
            // 2) 计算“本次UI操作”的增量角（对每根世界轴相互独立）
            Vector3 d;
            d.x = WrapDelta180(uiEuler.x - cache.eulerDeg.x);
            d.y = WrapDelta180(uiEuler.y - cache.eulerDeg.y);
            d.z = WrapDelta180(uiEuler.z - cache.eulerDeg.z);

            // 3) 世界轴左乘：谁变了应用谁（顺序不重要，因为是三根独立世界轴的微小增量；
            //    如果你希望严格模仿Unity，可按 X→Y→Z 顺序也行）
            if (std::abs(d.x) > 0.0001f) transform->RotateX(d.x);
            if (std::abs(d.y) > 0.0001f) transform->RotateY(d.y);
            if (std::abs(d.z) > 0.0001f) transform->RotateZ(d.z);
```
...
```cpp
        // --- Scale（保持原样）---
        Vector3 scale = transform->GetLocalScale();
        ImGui::Text("Scale       ");
        ImGui::SameLine(); ImGui::Text("X");
        ImGui::SameLine(); bool scaXChanged = ImGui::DragFloat("##scaX", &scale.x, 0.01f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Y");
        ImGui::SameLine(); bool scaYChanged = ImGui::DragFloat("##scaY", &scale.y, 0.01f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Z");
        ImGui::SameLine(); bool scaZChanged = ImGui::DragFloat("##scaZ", &scale.z, 0.01f, -FLT_MAX, FLT_MAX);
        if (scaXChanged || scaYChanged || scaZChanged) {
            transform->SetLocalScale(scale);
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
    using Mesh = EngineCore::Mesh;
    using Texture = EngineCore::Texture;
    //using MaterialMetaData = EngineCore::MaterialMetaData;
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