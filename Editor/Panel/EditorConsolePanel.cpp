#include "PreCompiledHeader.h"
#include "EditorConsolePanel.h"
#include "imgui.h"
#include "EditorSettings.h"

namespace EngineEditor
{
    // Console 透明度设置为0.0， 只用来接受以及处理一些SceneView事件
    void EditorConsolePanel::DrawGUI()
    {
        ImGui::SetNextWindowPos(ImVec2((float)EditorSettings::GetConsolePanelStartPos().x, (float)EditorSettings::GetConsolePanelStartPos().y));
        ImGui::SetNextWindowSize(ImVec2((float)EditorSettings::GetConsolePanelSize().x, (float)EditorSettings::GetConsolePanelSize().y));
        
        if (ImGui::Begin("Console", NULL, 
            ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoResize))
        {
        }
        ImGui::End();
    }
    
    EditorConsolePanel::~EditorConsolePanel()
    {
    }
}