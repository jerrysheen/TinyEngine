#include "PreCompiledHeader.h"
#include "ModelData.h"
#include "ModelUtils.h"


namespace EngineCore
{
    ModelData::ModelData(MetaData *metaData) : Resource(metaData)
    {
		ASSERT(metaData->dependentMap.size() == 0);

		ModelMetaData* modelMetaData = static_cast<ModelMetaData*>(metaData);
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(modelMetaData->path, 
			aiProcess_Triangulate 
			| aiProcess_JoinIdenticalVertices
			| aiProcess_ConvertToLeftHanded
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "Wrong Import!!!" << std::endl;
			ASSERT(false);
		}

		ProcessNode(scene->mRootNode, scene);

		//todo:: 自己判断layout加入
		layout.push_back(InputLayout(VertexAttribute::POSITION, 3 * sizeof(float), 3, 8 * sizeof(float), 0));
		layout.push_back(InputLayout(VertexAttribute::NORMAL, 3 * sizeof(float), 3, 8 * sizeof(float), 3 * sizeof(float)));
		layout.push_back(InputLayout(VertexAttribute::UV0, 2 * sizeof(float), 2, 8 * sizeof(float), 6 * sizeof(float)));

		RenderAPI::GetInstance().SetUpMesh(this, false);
    }



    void ModelData::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(aimesh, scene);
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	void ModelData::ProcessMesh(aiMesh* aiMesh, const aiScene* scene)
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
		vertex = std::move(vertexArray);


		//int step = indexBuffer.size()
		for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
		{
			aiFace face = aiMesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indexArray.push_back(face.mIndices[j]);
			}
		}
		index = std::move(indexArray);
	}

	// 走默认构造。
	ModelData::ModelData(Primitive primitiveType)
		:Resource()
    {
		switch (primitiveType)
		{
		case Primitive::Quad :
			ModelUtils::GetFullScreenQuad(this);	
			break;
		default:
			break;
		}
    }

}