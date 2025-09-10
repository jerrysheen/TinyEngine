#pragma once
#include "ModelData.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "Graphics/ModelData.h"

namespace EngineCore
{
    class ModelUtils
    {
    public:
        static ModelData* LoadMesh(const std::string& path);    
    private:
        static void ProcessNode(aiNode* node, const aiScene* scene, ModelData* mesh);
        static void ProcessMesh(aiMesh* aiMesh, const aiScene* scene, ModelData* mesh);
    };
}