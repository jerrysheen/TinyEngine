#include "PreCompiledHeader.h"
#include "EditorHierarchyPanel.h"
#include "EditorSettings.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
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
			auto* scene = EngineCore::SceneManager::GetInstance().GetCurrentScene();
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
        // 防止访问野指针或空指针
        if (gameObject == nullptr) return;
        if (gameObject->transform == nullptr) return;
        
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
				EditorGUIManager::GetInstance().SetCurrentSelected(gameObject);
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

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				selectedGO = gameObject;
				EditorGUIManager::GetInstance().SetCurrentSelected(gameObject);
			}

			if (nodeOpen)
			{
				for (auto subGameObject : gameObject->transform->childTransforms)
					DrawNode(subGameObject->gameObject);
				ImGui::TreePop();
			}
        }
    }
}