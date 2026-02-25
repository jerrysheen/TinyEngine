#include "PreCompiledHeader.h"
#include "MeshRenderer.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Material.h"
#include "GameObject.h"
#include "Resources/ResourceManager.h"
#include "Scene/GPUSCene.h"
#include "Renderer/RenderUniforms.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Renderer/BatchManager.h"

namespace EngineCore
{
	MeshRenderer::MeshRenderer(GameObject* go)
	{
		gameObject = go;
		// 设置为0意味着强制会有一次同步
		Scene* scene = go->GetOwnerScene();
		ASSERT(scene != nullptr);
		
		SetCPUWorldIndex(scene->CreateRenderNode());
		scene->MarkNodeCreated(this);
	}

    MeshRenderer::~MeshRenderer()
    {
		Scene* scene = gameObject->GetOwnerScene();
		ASSERT(scene != nullptr);

		scene->DeleteRenderNode(this);
	}

	void MeshRenderer::SetUpMaterialPropertyBlock()
    {
		ASSERT(mShardMatHandler.IsValid());
    }

	void MeshRenderer::SetSharedMaterial(const ResourceHandle<Material>& mat)
	{
		gameObject->GetOwnerScene()->MarkNodeMeshRendererDirty(this);
		mShardMatHandler = mat;
		//SetUpMaterialPropertyBlock();
	}

	Material* MeshRenderer::GetOrCreateMatInstance()
    {
		if(mInstanceMatHandler.IsValid())
		{
			return mInstanceMatHandler.Get();
		}
		ASSERT(mShardMatHandler.IsValid());
		mInstanceMatHandler = ResourceManager::GetInstance()->Instantiate<Material>(mShardMatHandler);
		ASSERT(mInstanceMatHandler.IsValid());
        return mInstanceMatHandler.Get();
    }

	ResourceHandle<Material> MeshRenderer::GetMaterial()
	{
		return HasMaterialOverride() ? mInstanceMatHandler : mShardMatHandler;
	}

    void MeshRenderer::UpdateBounds(const AABB &localBounds, const Matrix4x4 &worldMatrix)
    {
		worldBounds = localBounds;
		worldBounds.Transform(worldMatrix);
    }


    void MeshRenderer::OnLoadResourceFinished()
    {
		gameObject->GetOwnerScene()->MarkNodeMeshRendererDirty(this);
		return;
    }
}