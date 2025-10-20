#include "PreCompiledHeader.h"
#include "ModelUtils.h"
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
	// todo： vertex数据结构优化， 不需要三个数据
	void ModelUtils::GetFullScreenQuad(ModelData* modelData)
	{
		modelData->layout.emplace_back(InputLayout(VertexAttribute::POSITION, 3 * sizeof(float), 3, 8 * sizeof(float), 0));
		modelData->layout.emplace_back(InputLayout(VertexAttribute::NORMAL, 3 * sizeof(float), 3, 8 * sizeof(float), 3 * sizeof(float)));
		modelData->layout.emplace_back(InputLayout(VertexAttribute::UV0, 2 * sizeof(float), 2, 8 * sizeof(float), 6 * sizeof(float)));
		modelData->index.insert(modelData->index.end(), { 0,1,2,1,3,2 });
		modelData->vertex.insert(modelData->vertex.end(), {
			Vertex{Vector3(-1,-1,0), Vector3(0,0,1), Vector2(0,0)},  // 左下
			Vertex{Vector3(-1, 1,0), Vector3(0,0,1), Vector2(0,1)},  // 左上  
			Vertex{Vector3(1,-1,0), Vector3(0,0,1), Vector2(1,0)},  // 右下
			Vertex{Vector3(1, 1,0), Vector3(0,0,1), Vector2(1,1)}   // 右上
			});
		RenderAPI::GetInstance().SetUpMesh(modelData, false);
	}
}
