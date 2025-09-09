#include "PreCompiledHeader.h"
#include "ModelUtils.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


namespace EngineCore
{
    ModelData* ModelUtils::LoadMesh(const std::string& path)
    {
        Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "Wrong Import!!!" << std::endl;
			return false;
		}
        return nullptr;
    }

}
