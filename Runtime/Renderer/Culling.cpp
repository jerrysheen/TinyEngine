#include "PreCompiledHeader.h"
#include "Culling.h"
#include "Scene/SceneManager.h"
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
        auto* scene = SceneManager::GetInstance().GetCurrentScene();
        // todo: 快速的剔除逻辑， boudns逻辑
        for (auto* go : scene->objLists) 
        {
            auto* matComponent = go->GetComponent<MeshRenderer>();
            auto* modelComponent = go->GetComponent<MeshFilter>();
            if (matComponent != nullptr && modelComponent != nullptr) 
            {
                auto visibleItem = context.GetAvalileVisibleItem();
                visibleItem->mat = matComponent->mMatHandle.Get();
                visibleItem->model = modelComponent->mMeshHandle.Get();
                context.cameraVisibleItems.push_back(std::move(visibleItem));
            }
        }
    }

} // namespace EngineCore