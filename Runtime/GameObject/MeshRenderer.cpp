#include "PreCompiledHeader.h"
#include "MeshRenderer.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Material.h"
#include "GameObject.h"
#include "Serialization/ComponentFactory.h"
#include "Resources/ResourceManager.h"

REGISTER_SCRIPT(MeshRenderer)
namespace EngineCore
{
	MeshRenderer::MeshRenderer(GameObject* go)
	{
		gameObject = go;
	}

    void MeshRenderer::SetUpMaterialPropertyBlock()
    {
		ASSERT(mShardMatHandler.IsValid());
		Material* mat = mShardMatHandler.Get();
		mMaterialPropertyBlock.Initialize(mat->mShader->mShaderReflectionInfo.mPerDrawConstantBuffferReflectionInfo,
			mat->mShader->mShaderReflectionInfo.perDrawBufferSize);
    }


    Material* MeshRenderer::GetMaterial()
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
}