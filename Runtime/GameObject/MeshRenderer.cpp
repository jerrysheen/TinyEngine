#include "PreCompiledHeader.h"
#include "MeshRenderer.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Material.h"
#include "GameObject.h"

namespace EngineCore
{
	MeshRenderer::MeshRenderer(GameObject* gamObject)
	{
		mParentGO = gamObject;
	}
}