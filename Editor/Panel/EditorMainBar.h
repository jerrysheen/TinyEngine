#pragma once
#include "EditorPanel.h"
#include "Resources/ResourceManager.h"
#include "Resources/Asset.h"
#include "Resources/Resource.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "Graphics/Mesh.h"
#include "Graphics/Texture.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"


namespace EngineEditor
{
    using ResourceManager = EngineCore::ResourceManager;
    using AssetID = EngineCore::AssetID;
    using Resource = EngineCore::Resource;
    using AssetType = EngineCore::AssetType;
    using Material = EngineCore::Material;
    using Shader = EngineCore::Shader;
    using Mesh = EngineCore::Mesh;
    using Texture = EngineCore::Texture;
    //using MaterialMetaData = EngineCore::MaterialMetaData;
    using SceneManager = EngineCore::SceneManager;
    using Scene = EngineCore::Scene;

    class EditorMainBar : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorMainBar() override;
    private:
        void GenerateSceneMetaFile();
        void CreateHouseGameObject();

    };

}