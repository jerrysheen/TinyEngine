#include "PreCompiledHeader.h"
#include "Culling.h"
#include "Scene/SceneManager.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "GameObject/Transform.h"
#include "Scene/Scene.h"
#include "Math/Frustum.h"
#include "Core/PublicStruct.h"

namespace EngineCore
{
    void Culling::Run(Camera *cam, RenderContext &context)
    {
        auto* scene = SceneManager::GetInstance()->GetCurrentScene();
        auto& sceneRenderData = scene->renderSceneData;
        int totalVisible = 0;
        Vector3 campos = cam->gameObject->transform->GetWorldPosition();
        for(int i = 0; i < scene->renderSceneData.meshRendererList.size(); i++)
        {
            if( sceneRenderData.meshRendererList[i] &&
                sceneRenderData.meshFilterList[i])
            {
                if (cam->mFrustum.TestAABB(sceneRenderData.aabbList[i]) != IntersectResult::Outside) 
                {
                    RenderPacket packet;
                    packet.meshRenderer = sceneRenderData.meshRendererList[i];
                    packet.meshFilter = sceneRenderData.meshFilterList[i];
                    Vector3 pos = sceneRenderData.objectToWorldMatrixList[i].ExtractWorldPosition();
                    packet.distanToCamera = Vector3::Distance(campos, pos);
                    context.visibleItems.push_back(std::move(packet));
                    totalVisible++;
                }

            }
        }
        // todo: Culling::Run
        PROFILER_COUNTER_ADD("SceneItems", scene->allObjList.size());
        PROFILER_COUNTER_ADD("Visible Objects", totalVisible);

    }

} // namespace EngineCore