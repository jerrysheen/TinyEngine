#include "PreCompiledHeader.h"
#include "EditorHierarchyPanel.h"
#include "imgui.h"
#include "EditorSettings.h"

namespace EngineEditor
{
    void EditorHierarchyPanel::DrawGUI()
    {
        ImGui::SetNextWindowPos(ImVec2((float)EditorSettings::GetHierarchyPanelStartPos().x, (float)EditorSettings::GetHierarchyPanelStartPos().y));
        ImGui::SetNextWindowSize(ImVec2((float)EditorSettings::GetHierarchyPanelSize().x, (float)EditorSettings::GetHierarchyPanelSize().y));
        
		// ��������������
		if (ImGui::Begin("Hierarchy", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			//nodeIdx = 0;
			//auto scene = SceneManager::GetInstance()->GetCurScene();
			//for (auto gameObject : scene->gameObjects)
			//	DrawNode(gameObject);
		}
        ImGui::End();

    }

    EditorHierarchyPanel::~EditorHierarchyPanel()
    {
    }
}