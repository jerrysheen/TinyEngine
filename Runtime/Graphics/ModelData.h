#pragma once
#include "PreCompiledHeader.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Resources/MetaFile.h"
#include "Core/PublicStruct.h"
#include "Resources/Resource.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    ModelData* GetFullScreenQuad();
    class ModelData : public Resource
    {
    public:
        std::vector<Vertex> vertex;
        std::vector<int> index;
        std::vector<InputLayout> layout;
        ModelData() = default;
        ModelData(MetaData* metaData);
        ModelData(Primitive primitiveType);
    private:
        void ProcessNode(aiNode* node, const aiScene* scene);
        void ProcessMesh(aiMesh* aiMesh, const aiScene* scene);
    };

}