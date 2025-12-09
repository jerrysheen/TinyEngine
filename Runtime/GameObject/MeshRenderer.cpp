#include "PreCompiledHeader.h"
#include "MeshRenderer.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Material.h"
#include "GameObject.h"
#include "Serialization/ComponentFactory.h"
#include "Resources/ResourceManager.h"
#include "Graphics/GPUSceneManager.h"
#include "Renderer/RenderUniforms.h"


REGISTER_SCRIPT(MeshRenderer)
namespace EngineCore
{
	MeshRenderer::MeshRenderer(GameObject* go)
	{
		gameObject = go;
		perObjectDataAllocation = GPUSceneManager::GetInstance()->GetSinglePerObjectData();
	}

    MeshRenderer::~MeshRenderer()
    {
		GPUSceneManager::GetInstance()->RemoveSinglePerObjectData(perObjectDataAllocation);
    }

    void MeshRenderer::SetUpMaterialPropertyBlock()
    {
		ASSERT(mShardMatHandler.IsValid());
		Material* mat = mShardMatHandler.Get();
		mMaterialPropertyBlock.Initialize(mat->mShader->mShaderReflectionInfo.mPerDrawConstantBuffferReflectionInfo,
			mat->mShader->mShaderReflectionInfo.perDrawBufferSize);
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
		needUpdateWorldBounds = false;
    }

    void MeshRenderer::SyncPerObjectDataIfDirty()
    {
		if(!IsDirty) return;
		IsDirty = false;

		PerObjectData perObjectData;
		perObjectData.objectToWorld = gameObject->transform->GetWorldMatrix();
		GPUSceneManager::GetInstance()->UpdateSinglePerObjectData(perObjectDataAllocation, static_cast<void*>(&perObjectData));
    }
}