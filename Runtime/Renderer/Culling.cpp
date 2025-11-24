#include "PreCompiledHeader.h"
#include "Culling.h"
#include "Scene/SceneManager.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "GameObject/Transform.h"
#include "Scene/Scene.h"
#include "Math/Frustum.h"

namespace EngineCore
{
    void Culling::Run(Camera *cam, RenderContext &context)
    {
        // 从场景得到visibalItem， 然后组装
        // get some fake data....
        context.Reset();
        context.camera = cam;
        auto* scene = SceneManager::GetInstance()->GetCurrentScene();
        // todo: 快速的剔除逻辑， boudns逻辑
        for (auto* go : scene->allObjList)
        {
            auto* matComponent = go->GetComponent<MeshRenderer>();
            auto* modelComponent = go->GetComponent<MeshFilter>();
            auto* transformComponet = go->GetComponent<Transform>();
            if (matComponent != nullptr && modelComponent != nullptr) 
            {
                if (cam->mFrustum.TestAABB(matComponent->worldBounds) != IntersectResult::Outside) {

                    auto visibleItem = context.GetAvalileVisibleItem();

                    visibleItem->meshRenderer = matComponent;
                    visibleItem->meshFilter = modelComponent;
                    visibleItem->transform = transformComponet;
                    context.cameraVisibleItems.push_back(std::move(visibleItem));
                }
            }
        }
        // todo: Culling::Run
        PROFILER_COUNTER_ADD("SceneItems", scene->allObjList.size());
        PROFILER_COUNTER_ADD("Visible Objects", context.cameraVisibleItems.size());

    }

} // namespace EngineCore