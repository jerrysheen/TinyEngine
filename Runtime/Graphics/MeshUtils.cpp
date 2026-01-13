#include "PreCompiledHeader.h"
#include "MeshUtils.h"
#include "Renderer/RenderAPI.h"
#include "GeometryManager.h"
namespace EngineCore
{
	// todo： vertex数据结构优化， 不需要三个数据
	void MeshUtils::GetFullScreenQuad(Mesh* mesh)
	{
		mesh->layout.emplace_back(InputLayout(VertexAttribute::POSITION, 3 * sizeof(float), 3, 8 * sizeof(float), 0));
		mesh->layout.emplace_back(InputLayout(VertexAttribute::NORMAL, 3 * sizeof(float), 3, 8 * sizeof(float), 3 * sizeof(float)));
		mesh->layout.emplace_back(InputLayout(VertexAttribute::UV0, 2 * sizeof(float), 2, 8 * sizeof(float), 6 * sizeof(float)));
		mesh->index.insert(mesh->index.end(), { 0,1,2,1,3,2 });
		mesh->vertex.insert(mesh->vertex.end(), {
			Vertex{Vector3(-1,-1,0), Vector3(0,0,1), Vector2(0,0)},  // 左下
			Vertex{Vector3(-1, 1,0), Vector3(0,0,1), Vector2(0,1)},  // 左上  
			Vertex{Vector3(1,-1,0), Vector3(0,0,1), Vector2(1,0)},  // 右下
			Vertex{Vector3(1, 1,0), Vector3(0,0,1), Vector2(1,1)}   // 右上
			});
		mesh->UploadMeshToGPU();
	}
}
