#include "PreCompiledHeader.h"
#include "Culling.h"
#include "Scene/SceneManager.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "GameObject/Transform.h"
#include "Scene/Scene.h"
#include "Math/Frustum.h"
#include "Core/PublicStruct.h"
#include "Scene/CPUScene.h"
#include "Renderer/RenderEngine.h"
#include "Math/Math.h"

namespace EngineCore
{
    void Culling::Run(Camera *cam, RenderContext &context)
    {
        CPUScene scene = RenderEngine::GetInstance()->GetCPUScene();
        int totalVisible = 0;
        Vector3 campos = cam->gameObject->transform->GetWorldPosition();
        CPUSceneView sceneView = scene.GetSceneView();
        for(int i = 0; i < sceneView.materialList.size(); i++)
        {
            if( sceneView.materialList[i].IsValid() &&
                sceneView.meshList[i].IsValid())
            {
                if (cam->mFrustum.TestAABB(sceneView.worldBoundsList[i]) != IntersectResult::Outside)
                {
                    RenderPacket packet;
                    packet.materialID = sceneView.materialList[i];
                    packet.meshID = sceneView.meshList[i];
                    Vector3 pos = sceneView.objectToWorldList[i].ExtractWorldPosition();
                    packet.distanToCamera = Vector3::Distance(campos, pos);
                    packet.objectIndex = i;
                    context.visibleItems.push_back(std::move(packet));
                    totalVisible++;
                }

            }
        }
        // todo: Culling::Run
        PROFILER_COUNTER_ADD("SceneItems", scene.GetSceneView().worldBoundsList.size());
        PROFILER_COUNTER_ADD("Visible Objects", totalVisible);

    }

} // namespace EngineCore