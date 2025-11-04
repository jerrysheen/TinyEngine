#include "PreCompiledHeader.h"
#include "EditorMainBar.h"
#include "imgui.h"
#include "EditorSettings.h"

#include "Serialization/MetaData.h"
#include "Serialization/MetaSaver.h"
#include "Resources/ResourceHandle.h"

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
					if (ImGui::MenuItem("Generate Test MetaFile"))
					{
						GenerateTestMetaFile();
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

    void EditorMainBar::GenerateTestMetaFile()
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
		EngineCore::MetaSaver::SaveMetaData<EngineCore::MaterialMetaData>(&meta, meta.path);
	}

	void EditorMainBar::GenerateTextureMetaFile(const Texture* tex)
	{
	}

	void EditorMainBar::GenerateMeshMetaFile(const ModelData* data)
	{
	}

	void EditorMainBar::GenerateShaderMetaFile(const Shader* shader)
	{
	}

}