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
#include "Renderer/BatchManager.h"

REGISTER_SCRIPT(MeshRenderer)
namespace EngineCore
{
	MeshRenderer::MeshRenderer(GameObject* go)
	{
		gameObject = go;
		lastSyncTransformVersion = gameObject->transform->transformVersion;
		Scene* currentScene = SceneManager::GetInstance()->GetCurrentScene();
		if(currentScene != nullptr)
		{
			sceneRenderNodeIndex = currentScene->AddNewRenderNodeToCurrentScene(this);
		}
	}

    MeshRenderer::~MeshRenderer()
    {
		Scene* currentScene = SceneManager::GetInstance()->GetCurrentScene();
		if(currentScene != nullptr)
		{
			currentScene->DeleteRenderNodeFromCurrentScene(sceneRenderNodeIndex);
		}
		BatchManager::GetInstance()->TryDecreaseBatchCount(this);
	}

	void MeshRenderer::DeserializedFields(const json& data)
	{
		// 需要先序列化完Material，再去addBatch，不能在构造中ADD
		data.at("MatHandle").get_to(mShardMatHandler);
		SetUpMaterialPropertyBlock();
		BatchManager::GetInstance()->TryAddBatchCount(this);
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


}