#include "PreCompiledHeader.h"
#include "MeshFilter.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/ModelData.h"
#include "GameObject.h"

namespace EngineCore
{
	MeshFilter::MeshFilter(GameObject* go)
	{
		gameObject = go;
	}
}