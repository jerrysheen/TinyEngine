#include "PreCompiledHeader.h"
#include "MeshRenderer.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Material.h"
#include "GameObject.h"
#include "Serialization/ComponentFactory.h"
#include "Resources/ResourceManager.h"
#include "Graphics/GPUSceneManager.h"
#include "Renderer/RenderUniforms.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"

REGISTER_SCRIPT(MeshRenderer)
namespace EngineCore
{
	MeshRenderer::MeshRenderer(GameObject* go)
	{
		gameObject = go;
		perObjectDataAllocation = GPUSceneManager::GetInstance()->GetSinglePerObjectData();
		lastSyncTransformVersion = gameObject->transform->transformVersion;
		Scene* currentScene = SceneManager::GetInstance()->GetCurrentScene();
		if(currentScene != nullptr)
		{
			sceneRenderNodeIndex = currentScene->AddNewRenderNodeToCurrentScene(this);
		}
	}

    MeshRenderer::~MeshRenderer()
    {
		GPUSceneManager::GetInstance()->RemoveSinglePerObjectData(perObjectDataAllocation);
		Scene* currentScene = SceneManager::GetInstance()->GetCurrentScene();
		if(currentScene != nullptr)
		{
			currentScene->DeleteRenderNodeFromCurrentScene(sceneRenderNodeIndex);
		}
	}

    void MeshRenderer::SetUpMaterialPropertyBlock()
    {
		ASSERT(mShardMatHandler.IsValid());
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

    void MeshRenderer::SyncPerObjectDataIfDirty()
    {
		PerObjectData perObjectData;
		perObjectData.objectToWorld = gameObject->transform->GetWorldMatrix();
		BufferAllocation& allocation = GetMaterial()->materialAllocation;
		uint32_t matIndex = allocation.offset / allocation.size;
		perObjectData.matIndex = matIndex;
		GPUSceneManager::GetInstance()->UpdateSinglePerObjectData(perObjectDataAllocation, static_cast<void*>(&perObjectData));
    }
}