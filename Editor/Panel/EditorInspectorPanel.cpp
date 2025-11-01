#include "PreCompiledHeader.h"
#include "EditorInspectorPanel.h"
#include "imgui.h"
#include "EditorSettings.h"

namespace EngineEditor
{
    // Inspector 透明度设置为0.0， 只用来接受以及处理一些SceneView事件
    void EditorInspectorPanel::DrawGUI()
    {
        ImGui::SetNextWindowPos(ImVec2((float)EditorSettings::GetInspectorPanelStartPos().x, (float)EditorSettings::GetInspectorPanelStartPos().y));
        ImGui::SetNextWindowSize(ImVec2((float)EditorSettings::GetInspectorPanelSize().x, (float)EditorSettings::GetInspectorPanelSize().y));
        
        if (ImGui::Begin("Inspector", NULL, 
            ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoResize))
        {
        }
        ImGui::End();
    }
    
    EditorInspectorPanel::~EditorInspectorPanel()
    {
    }
}