#include "PreCompiledHeader.h"
#include "Culling.h"
#include "Managers/SceneManager.h"

namespace EngineCore
{
    void Culling::Run(Camera *cam, RenderContext &context)
    {
        // 从场景得到visibalItem， 然后组装
        // get some fake data....
        context.Reset();
        context.camera = cam;
        auto visibleItem = context.GetAvalileVisibleItem();
        visibleItem->mat = SceneManager::GetInstance().testMat.Get();
        visibleItem->model = SceneManager::GetInstance().testMesh.Get();
        context.cameraVisibleItems.push_back(std::move(visibleItem));
    }

} // namespace EngineCore