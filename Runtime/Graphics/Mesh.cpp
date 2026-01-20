#include "PreCompiledHeader.h"
#include "Mesh.h"
#include "MeshUtils.h"
#include "Resources/Asset.h"
#include "GeometryManager.h"
#include "Renderer/RenderAPI.h"


namespace EngineCore
{
    Mesh::Mesh(MetaData *metaData) : Resource(metaData)
    {
		mAssetType = AssetType::Mesh;
		ASSERT(metaData->dependentMap.size() == 0);

		ModelMetaData* MeshMetaData = static_cast<ModelMetaData*>(metaData);
		LoadAiMesh(MeshMetaData->path);
		UploadMeshToGPU();
    }


	void Mesh::LoadAiMesh(const string& path)
	{
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, 
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


	}

    void Mesh::UploadMeshToGPU()
    {
		if (vertex.size() == 0 || index.size() == 0) return;
		vertexAllocation = GeometryManager::GetInstance()->AllocateVertexBuffer(vertex.data(), vertex.size() * sizeof(Vertex));
		indexAllocation = GeometryManager::GetInstance()->AllocateIndexBuffer(index.data(), index.size() * sizeof(uint32_t));
		if(!isDynamic)
		{
			vertex.clear();
			vertex.shrink_to_fit();
			index.clear();
			index.shrink_to_fit();
		}
	}

    void Mesh::ProcessNode(aiNode *node, const aiScene *scene)
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

	void Mesh::ProcessMesh(aiMesh* aiMesh, const aiScene* scene)
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
			bounds.Encapsulate(currVertex.position);

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
	Mesh::Mesh(Primitive primitiveType)
		:Resource()
    {
		switch (primitiveType)
		{	
		case Primitive::Quad :
			MeshUtils::GetFullScreenQuad(this);
			break;
		case Primitive::Cube:
			LoadAiMesh(PathSettings::ResolveAssetPath("Model/Primitives/Cube.obj"));
			break;
		case Primitive::Sphere:
			LoadAiMesh(PathSettings::ResolveAssetPath("Model/Primitives/Sphere.obj"));
			break;
		default:
			break;
		}
		UploadMeshToGPU();
    }

}