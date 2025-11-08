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
					if (ImGui::MenuItem("Generate Scene MetaFile"))
					{
						GenerateSceneResourceMetaFile();
					}

					if (ImGui::MenuItem("Save SceneFile"))
					{

					}

					if (ImGui::MenuItem("TestSaveGO"))
					{
						TempTestGameObjectSerialization();
					}
					
					if (ImGui::MenuItem("LoadTestGO"))
					{
						TempTestGameObjectDeSerialization();
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

    void EditorMainBar::GenerateSceneResourceMetaFile()
    {
		auto resouceManager = ResourceManager::GetInstance();
        unordered_map<AssetID, Resource*> mResourceCache = resouceManager->mResourceCache;
		for(auto& [key, resource] : mResourceCache)
		{
			if(resource->GetPath() != "")
			{
				switch (resource->GetAssetType()) 
				{
				case AssetType::Material:
					GenerateMaterialMetaFile(static_cast<EngineCore::Material*>(resource));
					break;
				case AssetType::Mesh:
					GenerateMeshMetaFile(static_cast<EngineCore::ModelData*>(resource));
					break;
				case AssetType::Shader:
					GenerateShaderMetaFile(static_cast<EngineCore::Shader*>(resource));
					break;
				case AssetType::Texture2D:
					GenerateTextureMetaFile(static_cast<EngineCore::Texture*>(resource));
					break;
				}
			}
		}
    }

	void EditorMainBar::GenerateMaterialMetaFile(const Material* mat)
	{
		EngineCore::MaterialMetaData meta;
		meta.assetType = mat->GetAssetType();
		meta.path = mat->GetPath();
		// shader
		MetaData shadermeta;
		shadermeta.assetType = AssetType::Shader;
		shadermeta.path = mat->mShader->GetPath();
		meta.dependentMap.try_emplace("Shader", std::move(shadermeta));

		// texture dependency
		for (auto& [name, texture] : mat->mTexResourceMap) 
		{
			MetaData texmeta;
			texmeta.assetType = AssetType::Texture2D;
			if (auto* texHandle = std::get_if<EngineCore::ResourceHandle<Texture>>(&texture)) 
			{
				if (texHandle->IsValid()) 
				{
					texmeta.path = texHandle->Get()->GetPath();
					meta.dependentMap.try_emplace(name, std::move(texmeta));
					meta.textureData.try_emplace(name, nullptr);
				}
			}
		}

		meta.floatData = mat->mMaterialdata.floatData;
		meta.matrix4x4Data = mat->mMaterialdata.matrix4x4Data;
		meta.shaderPath = mat->mShader->GetPath();
		meta.vec2Data = mat->mMaterialdata.vec2Data;
		meta.vec3Data = mat->mMaterialdata.vec3Data;
		EngineCore::JsonSerializer::SvaeAsJson<EngineCore::MaterialMetaData>(&meta, meta.path);
	}

	void EditorMainBar::GenerateTextureMetaFile(const Texture* tex)
	{
		EngineCore::TextureMetaData meta;
		meta.assetType = tex->GetAssetType();
		meta.path = tex->GetPath();
		meta.dimension = EngineCore::TextureDimension::TEXTURE2D;
		meta.format = EngineCore::TextureFormat::R8G8B8A8;
		meta.height = tex->mHeight;
		meta.width = tex->mWidth;
		meta.path = tex->GetPath();
		EngineCore::JsonSerializer::SvaeAsJson<EngineCore::TextureMetaData>(&meta, meta.path);
	}

	void EditorMainBar::GenerateMeshMetaFile(const ModelData* data)
	{
		EngineCore::ModelMetaData meta;
		meta.assetType = data->GetAssetType();
		meta.path = data->GetPath();
		meta.path = data->GetPath();
		EngineCore::JsonSerializer::SvaeAsJson<EngineCore::ModelMetaData>(&meta, meta.path);
	}

	void EditorMainBar::GenerateShaderMetaFile(const Shader* shader)
	{
		EngineCore::MetaData meta;
		meta.assetType = shader->GetAssetType();
		meta.path = shader->GetPath();
		meta.path = shader->GetPath();
		EngineCore::JsonSerializer::SvaeAsJson<EngineCore::MetaData>(&meta, meta.path);
	}

    void EditorMainBar::TempTestGameObjectSerialization()
    {
		EngineCore::GameObject* house = EngineCore::SceneManager::GetInstance()->FindGameObject("house");
		ASSERT(house != nullptr);
		
		EngineCore::JsonSerializer::SvaeAsJson<EngineCore::GameObject>(house, "/Temp/house.meta");
	}

    void EditorMainBar::TempTestGameObjectDeSerialization()
    {

		json j = EngineCore::JsonSerializer::ReadFromJson("/Temp/house.meta");
		// 使用MetaFactory从JSON创建GameObject
		EngineCore::GameObject* go = EngineCore::MetaFactory::CreateGameObjectFromMeta(j);
		EngineCore::SceneManager::GetInstance()->GetCurrentScene()->AddGameObjectToSceneList(go);
    }

    void EditorMainBar::GenerateSceneMetaFile()
	{
		auto* scene = SceneManager::GetInstance()->GetCurrentScene();
		for (auto* go : scene->allObjList) 
		{
			EngineCore::JsonSerializer::SvaeAsJson<EngineCore::GameObject>(go, "F:/" + go->name + ".meta");
		}
	}

}