#include "PreCompiledHeader.h"
#include "MeshFilter.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Mesh.h"
#include "GameObject.h"
#include "Serialization/ComponentFactory.h"
#include "Renderer/BatchManager.h"

REGISTER_SCRIPT(MeshFilter)
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

	void MeshFilter::DeserializedFields(const json& data)
	{
		data.at("MeshHandle").get_to(mMeshHandle);
		BatchManager::GetInstance()->TryAddBatchCount(this);
	}
}