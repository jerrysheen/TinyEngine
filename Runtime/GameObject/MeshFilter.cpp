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
	MeshFilter::MeshFilter(GameObject* go)
	{
		gameObject = go;
		BatchManager::GetInstance()->TryAddBatchCount(this);
	}

    MeshFilter::~MeshFilter()
    {
		BatchManager::GetInstance()->TryDecreaseBatchCount(this);

    }
	void MeshFilter::DeserializedFields(const json& data)
	{
		data.at("MeshHandle").get_to(mMeshHandle);
		BatchManager::GetInstance()->TryAddBatchCount(this);
	}
}