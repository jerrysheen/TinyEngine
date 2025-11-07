#pragma once
#include "EditorPanel.h"
#include "Serialization/MetaSaver.h"
#include "Resources/ResourceManager.h"
#include "Resources/Asset.h"
#include "Resources/Resource.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "Graphics/ModelData.h"
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
    using ModelData = EngineCore::ModelData;
    using Texture = EngineCore::Texture;
    //using MaterialMetaData = EngineCore::MaterialMetaData;
    using TextureMetaData = EngineCore::TextureMetaData;
    using MetaData = EngineCore::MetaData;
    using SceneManager = EngineCore::SceneManager;
    using Scene = EngineCore::Scene;

    class EditorMainBar : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorMainBar() override;
    private:
        void GenerateSceneResourceMetaFile();
        void GenerateSceneMetaFile();
        void GenerateMaterialMetaFile(const Material* mat);
        void GenerateTextureMetaFile(const Texture* tex);
        void GenerateMeshMetaFile(const ModelData* data);
        void GenerateShaderMetaFile(const Shader* shader);

        void TempTestGameObjectSerialization();
        void TempTestGameObjectDeSerialization();
    };

}