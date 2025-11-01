#include "PreCompiledHeader.h"
#include "EditorMainBar.h"
#include "imgui.h"
#include "EditorSettings.h"

namespace EngineEditor
{
    void EditorMainBar::DrawGUI()
    {

		ImGui::SetNextWindowPos(ImVec2((float)EditorSettings::GetMainBarPanelStartPos().x, (float)EditorSettings::GetMainBarPanelStartPos().y));
		ImGui::SetNextWindowSize(ImVec2((float)EditorSettings::GetMainBarPanelSize().x, (float)EditorSettings::GetMainBarPanelSize().y));
		

		if (ImGui::Begin("TinyEngine", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar))
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New Scene")){}
						//EditorDialogBoxManager::GetInstance()->PopMessage("Notice", "This feature is not implemented yet.");
					if (ImGui::MenuItem("Open Scene")){}
						//EditorDialogBoxManager::GetInstance()->PopMessage("Notice", "This feature is not implemented yet.");
					if (ImGui::MenuItem("Save Scene")){}
						//EditorDialogBoxManager::GetInstance()->PopMessage("Notice", "This feature is not implemented yet.");

					ImGui::Separator();

					if (ImGui::MenuItem("New Project")){}
						//EditorDialogBoxManager::GetInstance()->PopMessage("Notice", "This feature is not implemented yet.");
					if (ImGui::MenuItem("Open Project")){}
						//EditorDialogBoxManager::GetInstance()->PopMessage("Notice", "This feature is not implemented yet.");
					if (ImGui::MenuItem("Save Project")){}
						//EditorDialogBoxManager::GetInstance()->PopMessage("Notice", "This feature is not implemented yet.");

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Edit"))
				{
					if (ImGui::MenuItem("Project Settings")) {}
						//EditorDialogBoxManager::GetInstance()->PopMessage("Notice", "This feature is not implemented yet.");

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Assets"))
				{
					if (ImGui::MenuItem("Compile All Shader for Vulkan"))
					{
					}

					if (ImGui::MenuItem("Compile All Shader for DirectX12"))
					{
					}

					if (ImGui::MenuItem("Generate HLSL for DirectX12"))
					{
					}

					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

		}
		ImGui::End();
    }
    
    EditorMainBar::~EditorMainBar()
    {
    }
}