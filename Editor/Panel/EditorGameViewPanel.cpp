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
        

        if (ImGui::Begin("GameView", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoBackground)) // 依然保持透明
        {
            const float padding = 10.0f;
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImVec2 overlaySize(230.0f, 100.0f);

            // 右上角
            ImGui::SetCursorPos(ImVec2(
                avail.x - overlaySize.x - padding,
                padding));

            // 改变样式：深色背景 + 白色文字 + 明显边框
            ImGuiStyle& style = ImGui::GetStyle();
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.10f, 0.10f, 0.8f)); // 深灰半透明
            ImGui::PushStyleColor(ImGuiCol_Border,  ImVec4(1.0f, 1.0f, 1.0f, 0.6f));   // 边框颜色
            ImGui::PushStyleColor(ImGuiCol_Text,    ImVec4(1.0f, 1.0f, 1.0f, 1.0f));   // 白字
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);                  // 有边框

            ImGui::BeginChild("GameViewStats", overlaySize, true,
                            ImGuiWindowFlags_NoScrollbar);

            ImGui::Text("Draws: %u",  2);
            ImGui::Text("Visible: %u", 1);
            ImGui::Text("Culled: %u",  3);
            ImGui::Text("SRP Batches: %u", 1);
            ImGui::Text("FPS: %.2f", EngineCore::Profiler::Get().GetFPS());

            ImGui::EndChild();

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
        }
        ImGui::End();
    }
    
    EditorGameViewPanel::~EditorGameViewPanel()
    {
    }
}