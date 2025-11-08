#include "PreCompiledHeader.h"
#include "MeshRenderer.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Material.h"
#include "GameObject.h"
#include "Serialization/ComponentFactory.h"


REGISTER_SCRIPT(MeshRenderer)
namespace EngineCore
{
	MeshRenderer::MeshRenderer(GameObject* go)
	{
		gameObject = go;
	}
}