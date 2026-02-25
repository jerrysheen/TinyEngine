#include "PreCompiledHeader.h"
#include "MeshFilter.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Mesh.h"
#include "GameObject.h"
#include "Renderer/BatchManager.h"
#include "Scene/Scene.h"

namespace EngineCore
{
	// 延迟， meshfilter，meshrenderer绑定的时候都会尝试TryAddBatch，
	// 确保数据都在
	MeshFilter::MeshFilter(GameObject* go)
	{
		gameObject = go;
	}


    MeshFilter::~MeshFilter()
    {

    }

	void MeshFilter::OnLoadResourceFinished()
	{
		Scene* scene = this->gameObject->GetOwnerScene();
		ASSERT(scene != nullptr);
		scene->MarkNodeRenderableDirty(this->gameObject);
		return;
	}

}