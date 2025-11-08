#include "PreCompiledHeader.h"
#include "MeshFilter.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/ModelData.h"
#include "GameObject.h"
#include "Serialization/ComponentFactory.h"


REGISTER_SCRIPT(MeshFilter)
namespace EngineCore
{
	MeshFilter::MeshFilter(GameObject* go)
	{
		gameObject = go;
	}
}