#include "PreCompiledHeader.h"
#include "MeshFilter.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Mesh.h"
#include "GameObject.h"
#include "Renderer/BatchManager.h"

namespace EngineCore
{
	// 延迟， meshfilter，meshrenderer绑定的时候都会尝试TryAddBatch，
	// 确保数据都在
	MeshFilter::MeshFilter(GameObject* go)
	{
		gameObject = go;
		BatchManager::GetInstance()->TryAddBatchCount(this);
	}


    MeshFilter::~MeshFilter()
    {
		BatchManager::GetInstance()->TryDecreaseBatchCount(this);

    }

	void MeshFilter::OnLoadResourceFinished()
	{
		this->gameObject->transform->transformVersion++;
		return;
	}

}