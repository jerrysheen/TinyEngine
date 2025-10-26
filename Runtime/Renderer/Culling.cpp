#include "PreCompiledHeader.h"
#include "Culling.h"
#include "Managers/SceneManager.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"

namespace EngineCore
{
    void Culling::Run(Camera *cam, RenderContext &context)
    {
        // 从场景得到visibalItem， 然后组装
        // get some fake data....
        context.Reset();
        context.camera = cam;
        auto visibleItem = context.GetAvalileVisibleItem();
        visibleItem->mat = SceneManager::GetInstance().mTestGameObject->GetComponent<MeshRenderer>()->mMatHandle.Get();
        visibleItem->model = SceneManager::GetInstance().mTestGameObject->GetComponent<MeshFilter>()->mMeshHandle.Get();
        context.cameraVisibleItems.push_back(std::move(visibleItem));
    }

} // namespace EngineCore