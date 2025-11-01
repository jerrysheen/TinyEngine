#include "PreCompiledHeader.h"
#include "EditorGameViewPanel.h"
#include "imgui.h"
#include "EditorSettings.h"

namespace EngineEditor
{
    // GameView 透明度设置为0.0， 只用来接受以及处理一些SceneView事件
    void EditorGameViewPanel::DrawGUI()
    {
        ImGui::SetNextWindowPos(ImVec2((float)EditorSettings::GetGameViewPanelStartPos().x, (float)EditorSettings::GetGameViewPanelStartPos().y));
        ImGui::SetNextWindowSize(ImVec2((float)EditorSettings::GetGameViewPanelSize().x, (float)EditorSettings::GetGameViewPanelSize().y));
        
        if (ImGui::Begin("GameView", NULL, 
            ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoBackground))  // 这个标志会自动使背景透明
        {
        }
        ImGui::End();
    }
    
    EditorGameViewPanel::~EditorGameViewPanel()
    {
    }
}