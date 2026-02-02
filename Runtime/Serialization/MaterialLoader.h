#pragma once
#include "AssetHeader.h"
#include "StreamHelper.h"
#include "Resources/IResourceLoader.h"
#include "Graphics/Material.h"
#include "MaterialLibrary/MaterialLayout.h"
#include "MaterialLibrary/MaterialInstance.h"
#include "MaterialLibrary/MaterialArchetypeRegistry.h"
#include "Settings/ProjectSettings.h"
#include "Graphics/Texture.h"
#include "Resources/ResourceManager.h"
#include <vector>
#include <cstdio>

namespace EngineCore
{
    struct alignas(16) MetaMaterialHeader
    {
        bool     enable;
        uint64_t shaderID;
    };

    struct alignas(16) MetatextureDependency
    {
        char name[50];
        uint64_t ASSETID = 0;
    };

    struct alignas(16) MetaTextureToBindlessBlockIndex
    {
        char name[50];
        uint32_t offset;
    };

    class MaterialLoader: public IResourceLoader                       
    {
    public:
        virtual ~MaterialLoader() = default;
        virtual LoadResult Load(const std::string& relativePath) override
        {
            LoadResult result;
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream in(path, std::ios::binary);
            in.seekg(sizeof(AssetHeader));
            bool isBindless = false;
            StreamHelper::Read(in, isBindless);

            std::string archyTypeName;
            StreamHelper::ReadString(in, archyTypeName);
            if (archyTypeName.empty())
            {
                archyTypeName = "StandardPBR";
            }

            std::string shaderPath;
            if (archyTypeName == "StandardPBR")
            {
                shaderPath = RenderSettings::s_EnableVertexPulling
                    ? "Shader/StandardPBR_VertexPulling.hlsl"
                    : "Shader/StandardPBR.hlsl";
            }

            ASSERT(!shaderPath.empty());

            Material* mat = new Material();
            mat->SetAssetCreateMethod(AssetCreateMethod::Serialization);
            mat->SetPath(relativePath);
            mat->SetAssetID(AssetIDGenerator::NewFromFile(relativePath));
            mat->isBindLessMaterial = isBindless;
            mat->archyTypeName = archyTypeName;

            uint64_t assetPathID = AssetRegistry::GetInstance()->GetAssetIDFromPath(shaderPath);
            result.dependencyList.emplace_back
            (
                assetPathID,
                AssetType::Shader,
                nullptr
            );
            mat->mShader = ResourceHandle<Shader>(assetPathID);

            std::vector<MetatextureDependency> textureDependencyList;
            StreamHelper::ReadVector(in, textureDependencyList);
            for(auto& textureDependency : textureDependencyList)
            {
                std::string texName = std::string(textureDependency.name);
                mat->textureData[texName] = nullptr;
                result.dependencyList.emplace_back
                    (
                        textureDependency.ASSETID, 
                        AssetType::Texture2D,
                        [=](){
                            ResourceHandle<Texture> texID(textureDependency.ASSETID);
                            mat->SetTexture(texName, texID);
                        }
                    );
                mat->SetTexture(texName, ResourceHandle<Texture>(ResourceManager::GetInstance()->mDefaultTexture->GetAssetID()));
            }

            mat->matInstance = std::make_unique<MaterialInstance>(MaterialArchetypeRegistry::GetInstance().GetArchytypeLayout(mat->archyTypeName));

            std::vector<uint8_t> materialInstanceData;
            StreamHelper::ReadVector(in, materialInstanceData);
            mat->matInstance->SetInstanceData(materialInstanceData);

            if(mat->isBindLessMaterial)
            {
                std::vector<MetaTextureToBindlessBlockIndex> textureToBinlessOffsetList;
                StreamHelper::ReadVector(in, textureToBinlessOffsetList);
                for(auto& textureToBindLessBlockIndex : textureToBinlessOffsetList)
                {
                    mat->matInstance->GetLayout().textureToBlockIndexMap[textureToBindLessBlockIndex.name] = textureToBindLessBlockIndex.offset; 
                }
            }



            result.resource = mat;
            return result;
        }

        void SaveMaterialToBin(const Material* mat, const std::string& relativePath, uint64_t id)
        {
            std::string binPath = PathSettings::ResolveAssetPath(relativePath);
            std::ofstream out(binPath, std::ios::binary);

            AssetHeader header;
            header.assetID =id;
            header.type = 3;
            StreamHelper::Write(out, header);

            StreamHelper::Write(out, mat->isBindLessMaterial);
            StreamHelper::WriteString(out, mat->archyTypeName);
            std::vector<MetatextureDependency> textureDependencyList;
            for(auto& [key, value] : mat->textureHandleMap)
            {
                MetatextureDependency currTex;
                std::sprintf(currTex.name, key.c_str(), 50);
                std::snprintf(currTex.name, sizeof(currTex.name), "%s", key.c_str());
                currTex.ASSETID = value.GetAssetID();
                textureDependencyList.push_back(currTex);
            }
            StreamHelper::WriteVector(out, textureDependencyList);

            std::vector<uint8_t> materialInstanceData = mat->matInstance->GetInstanceData();
            StreamHelper::WriteVector(out, materialInstanceData);

            // 记录bindless插槽索引
            if(mat->isBindLessMaterial)
            {
                std::vector<MetaTextureToBindlessBlockIndex> textureToBinlessOffsetList;
                for(auto& [key, value] : mat->matInstance->GetLayout().textureToBlockIndexMap)
                {
                    MetaTextureToBindlessBlockIndex currTextureToIndex;
                    std::snprintf(currTextureToIndex.name, sizeof(currTextureToIndex.name), "%s", key.c_str());
                    currTextureToIndex.offset = value;
                    textureToBinlessOffsetList.push_back(currTextureToIndex);
                }
                StreamHelper::WriteVector(out, textureToBinlessOffsetList);
            }
        }
    };
};
