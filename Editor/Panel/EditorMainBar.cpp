#include "PreCompiledHeader.h"
#include "EditorMainBar.h"
#include "imgui.h"
#include "EditorSettings.h"

#include "Serialization/MetaData.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/MetaFactory.h"
#include "Resources/ResourceHandle.h"
#include "Settings/ProjectSettings.h"


namespace EngineEditor
{
	using json = nlohmann::json;

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


					if (ImGui::MenuItem("Save SceneFile"))
					{
						GenerateSceneMetaFile();
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

    void EditorMainBar::GenerateSceneMetaFile()
	{
		auto* scene = SceneManager::GetInstance()->GetCurrentScene();
		EngineCore::JsonSerializer::SvaeAsJson<EngineCore::Scene>(scene, "Scenes/" + scene->name);
	}

}