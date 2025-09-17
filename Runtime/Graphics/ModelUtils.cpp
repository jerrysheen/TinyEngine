#include "PreCompiledHeader.h"
#include "ModelUtils.h"
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
	//todo: 目前只支持单模型导入，不支持submesh。
    ModelData* ModelUtils::LoadMesh(const std::string& path)
    {
        Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "Wrong Import!!!" << std::endl;
			return false;
		}

		ModelData* meshData = new ModelData();
		
		ProcessNode(scene->mRootNode, scene, meshData);

		RenderAPI::GetInstance().SetUpMesh(meshData, false);
        return meshData;
    }

	void ModelUtils::ProcessNode(aiNode* node, const aiScene* scene, ModelData* mesh)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(aimesh, scene, mesh);
		}
		
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, mesh);
		}
	}
	
	void ModelUtils::ProcessMesh(aiMesh* aiMesh, const aiScene* scene, ModelData* mesh)
	{
		std::vector<Vertex> vertexArray;
		std::vector<int> indexArray;
		std::vector<InputLayout> inputLayoutArray;
		Vertex currVertex;
		int offset = 0;
		int stride = 0;
		for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
		{
			if (aiMesh->mVertices)
			{
				currVertex.position.x = (aiMesh->mVertices[i].x);
				currVertex.position.y = (aiMesh->mVertices[i].y);
				currVertex.position.z = (aiMesh->mVertices[i].z);
			}
			else
			{
				currVertex.position.x = 0;
				currVertex.position.y = 0;
				currVertex.position.z = 0;
			}

			if (aiMesh->mNormals) 
			{
				currVertex.normal.x = (aiMesh->mNormals[i].x);
				currVertex.normal.y = (aiMesh->mNormals[i].y);
				currVertex.normal.z = (aiMesh->mNormals[i].z);
			}
			else
			{
				currVertex.normal.x = 0;
				currVertex.normal.y = 0;
				currVertex.normal.z = 0;
			}

			if (aiMesh->mTextureCoords[0])
			{
				currVertex.uv.x = (aiMesh->mTextureCoords[0][i].x);
				currVertex.uv.y = (aiMesh->mTextureCoords[0][i].y);
			}
			else
			{
				currVertex.uv.x = 0;
				currVertex.uv.y = 0;
			}
			vertexArray.push_back(currVertex);
		}
		mesh->vertex = std::move(vertexArray);

		inputLayoutArray.push_back(InputLayout(VertexAttribute::POSITION, 3 * sizeof(float), 3, 8 * sizeof(float), 0));
		inputLayoutArray.push_back(InputLayout(VertexAttribute::NORMAL, 3 * sizeof(float), 3, 8 * sizeof(float), 3 * sizeof(float)));
		inputLayoutArray.push_back(InputLayout(VertexAttribute::UV0, 2 * sizeof(float), 2, 8 * sizeof(float), 6 * sizeof(float)));
		mesh->layout = std::move(inputLayoutArray);
		//int step = indexBuffer.size()
		for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
		{
			aiFace face = aiMesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indexArray.push_back(face.mIndices[j]);
			}
		}
		mesh->index = std::move(indexArray);
	}

}
