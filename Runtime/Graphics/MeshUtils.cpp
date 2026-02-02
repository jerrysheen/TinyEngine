#include "PreCompiledHeader.h"
#include "MeshUtils.h"
#include "Renderer/RenderAPI.h"
#include "GeometryManager.h"
namespace EngineCore
{
	// todo： vertex数据结构优化， 不需要三个数据
	void MeshUtils::GetFullScreenQuad(Mesh* mesh)
	{
		const int vertexStride = sizeof(Vertex);
		mesh->layout.emplace_back(InputLayout(VertexAttribute::POSITION, sizeof(Vector3), 3, vertexStride, 0));
		mesh->layout.emplace_back(InputLayout(VertexAttribute::NORMAL, sizeof(Vector3), 3, vertexStride, sizeof(Vector3)));
		mesh->layout.emplace_back(InputLayout(VertexAttribute::UV0, sizeof(Vector2), 2, vertexStride, sizeof(Vector3) * 2));
		mesh->layout.emplace_back(InputLayout(VertexAttribute::TANGENT, sizeof(Vector4), 4, vertexStride, sizeof(Vector3) * 2 + sizeof(Vector2)));
		mesh->index.insert(mesh->index.end(), { 0,1,2,1,3,2 });
		mesh->vertex.insert(mesh->vertex.end(), {
			Vertex{Vector3(-1,-1,0), Vector3(0,0,1), Vector2(0,0), Vector4(1,0,0,1)},  // 左下
			Vertex{Vector3(-1, 1,0), Vector3(0,0,1), Vector2(0,1), Vector4(1,0,0,1)},  // 左上  
			Vertex{Vector3(1,-1,0), Vector3(0,0,1), Vector2(1,0), Vector4(1,0,0,1)},  // 右下
			Vertex{Vector3(1, 1,0), Vector3(0,0,1), Vector2(1,1), Vector4(1,0,0,1)}   // 右上
			});
		mesh->UploadMeshToGPU();
	}
}
