#include "PreCompiledHeader.h"
#include "EditorProjectPanel.h"
#include "imgui.h"
#include "EditorSettings.h"

namespace EngineEditor
{
    // Project 透明度设置为0.0， 只用来接受以及处理一些SceneView事件
    void EditorProjectPanel::DrawGUI()
    {
        ImGui::SetNextWindowPos(ImVec2((float)EditorSettings::GetProjectPanelStartPos().x, (float)EditorSettings::GetProjectPanelStartPos().y));
        ImGui::SetNextWindowSize(ImVec2((float)EditorSettings::GetProjectPanelSize().x, (float)EditorSettings::GetProjectPanelSize().y));
        
        if (ImGui::Begin("Project", NULL, 
            ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoResize))
        {
        }
        ImGui::End();
    }
    
    EditorProjectPanel::~EditorProjectPanel()
    {
    }
}