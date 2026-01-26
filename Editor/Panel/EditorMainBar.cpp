#include "PreCompiledHeader.h"
#include "EditorMainBar.h"
#include "imgui.h"
#include "EditorSettings.h"

#include "Resources/ResourceHandle.h"
#include "Settings/ProjectSettings.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "GameObject/Transform.h"
#include "Resources/ResourceManager.h"
#include "Scene/BistroSceneLoader.h"
//#include "Settings/PathSettings.h"


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


				if (ImGui::MenuItem("Save SceneFile"))
				{
					GenerateSceneMetaFile();
				}

				if (ImGui::MenuItem("Serialize Bistro"))
				{
					using namespace EngineCore;
					auto* scene = SceneManager::GetInstance()->GetCurrentScene();
					if (scene) {
						// Save to binary cache
						std::string bistroPath = PathSettings::ResolveAssetPath("/Scenes/niagara_bistro/bistro.gltf.bin");
						BistroSceneLoader::SaveToCache(scene, bistroPath);
					}
				}


				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("GameObject"))
			{
				if (ImGui::MenuItem("Create House"))
				{
					CreateHouseGameObject();
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
	}

	void EditorMainBar::CreateHouseGameObject()
	{
		//using namespace EngineCore;

		//auto* scene = SceneManager::GetInstance()->GetCurrentScene();
		//if (!scene)
		//{
		//	// 可以弹出错误提示
		//	return;
		//}

		//// 创建房子GameObject
		//auto* houseObj = scene->CreateGameObject("House");

		//// 添加MeshFilter组件
		//auto* meshFilter = houseObj->AddComponent<MeshFilter>();
		//meshFilter->mMeshHandle = ResourceManager::GetInstance()->LoadAsset<Mesh>("Model/viking_room.obj");

		//// 添加MeshRenderer组件
		//auto* meshRenderer = houseObj->AddComponent<MeshRenderer>();
		//meshRenderer->SetSharedMaterial( ResourceManager::GetInstance()->LoadAsset<Material>("Material/testMat.mat"));

		//// 设置Transform
		//auto* transform = houseObj->GetComponent<Transform>();
		//transform->RotateX(90.0f);
		//transform->RotateY(135.0f);
		////transform->UpdateNow();

		//// 更新材质的世界矩阵
		////meshRenderer->mMatHandle.Get()->SetMatrix4x4("WorldMatrix", transform->GetWorldMatrix());
	}

}