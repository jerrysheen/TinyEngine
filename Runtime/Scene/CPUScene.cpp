#include "PreCompiledHeader.h"
#include "CPUScene.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "Math/Math.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Mesh.h"
#include "SceneStruct.h"
#include "Renderer/BatchManager.h"

namespace EngineCore
{
    void CPUScene::Update(uint32_t frameID)
    {
        mCurrentFrame = frameID;
    }

    void CPUScene::CreateRenderNode(uint32_t renderID, NodeDirtyPayload &payload)
    {
        EnsureCapacity(renderID);
        ASSERT(payload.materialID.IsValid());
        materialList[renderID] = payload.materialID;
        objectToWorldMatrixList[renderID] = payload.transform->GetWorldMatrix();
        
        if(!payload.meshID.IsValid()) return;
        Mesh* mesh = ResourceManager::GetInstance()->GetResource<Mesh>(payload.meshID);
        localBoundCacheList[renderID] = mesh->bounds;
        worldBoundsList[renderID] = localBoundCacheList[renderID];
        worldBoundsList[renderID].Transform(objectToWorldMatrixList[renderID]);
        meshList[renderID] = payload.meshID;
        layerList[renderID] = 1;
        BatchManager::GetInstance()->TryAddBatches(meshList[renderID], materialList[renderID], layerList[renderID]);
        // todo : Add payload
        return;
    }

    void CPUScene::OnRenderNodeMaterialDirty(uint32_t renderID, NodeDirtyPayload &payload)
    {
        ASSERT(materialList.size() > renderID);
        ASSERT(payload.materialID.IsValid());
        BatchManager::GetInstance()->TryDecreaseBatches(meshList[renderID], materialList[renderID], layerList[renderID]);
        materialList[renderID] = payload.materialID;
        BatchManager::GetInstance()->TryAddBatches(meshList[renderID], materialList[renderID], layerList[renderID]);

        return;
    }

    void CPUScene::OnRenderNodeTransformDirty(uint32_t renderID, NodeDirtyPayload &payload)
    {
        ASSERT(materialList.size() > renderID);
        objectToWorldMatrixList[renderID] = payload.transform->GetWorldMatrix();
        worldBoundsList[renderID].Transform(objectToWorldMatrixList[renderID]);
        return;
    }

    void CPUScene::OnRenderNodeMeshDirty(uint32_t renderID, NodeDirtyPayload &payload)
    {
        ASSERT(materialList.size() > renderID);
        BatchManager::GetInstance()->TryDecreaseBatches(meshList[renderID], materialList[renderID], layerList[renderID]);
        Mesh* mesh = ResourceManager::GetInstance()->GetResource<Mesh>(payload.meshID);
        localBoundCacheList[renderID] = mesh->bounds;
        worldBoundsList[renderID] = localBoundCacheList[renderID];
        worldBoundsList[renderID].Transform(objectToWorldMatrixList[renderID]);
        meshList[renderID] = payload.meshID;
        BatchManager::GetInstance()->TryAddBatches(meshList[renderID], materialList[renderID], layerList[renderID]);

        return;
    }


    CPUSceneView CPUScene::GetSceneView()
    {
        return CPUSceneView(objectToWorldMatrixList,
            worldBoundsList,
            materialList,
            meshList,
            layerList
            );
    }

    void CPUScene::DeleteRenderNode(uint32_t renderID)
    {
        ASSERT(materialList.size() > renderID);
        BatchManager::GetInstance()->TryDecreaseBatches(meshList[renderID], materialList[renderID], layerList[renderID]);
        materialList[renderID].SetInValid();
        meshList[renderID].SetInValid();
        worldBoundsList[renderID] = AABB();
        localBoundCacheList[renderID] = AABB();
        objectToWorldMatrixList[renderID] = Matrix4x4::Identity;

    }

    void CPUScene::ApplyDirtyNode(uint32_t renderID, NodeDirtyFlags cpuWorldRenderNodeFlag, NodeDirtyPayload &cpuWorldRenderNodePayload)
    {
        EnsureCapacity(renderID);
        if((uint32_t)cpuWorldRenderNodeFlag & (uint32_t)NodeDirtyFlags::Created)
        {
            CreateRenderNode(renderID, cpuWorldRenderNodePayload);
            return;
        }

        if((uint32_t)cpuWorldRenderNodeFlag & (uint32_t)NodeDirtyFlags::Destory)
        {
            DeleteRenderNode(renderID);
            return;
        }

        if((uint32_t)cpuWorldRenderNodeFlag & (uint32_t)NodeDirtyFlags::TransformDirty)
        {
            OnRenderNodeTransformDirty(renderID, cpuWorldRenderNodePayload);
        }
        
        if((uint32_t)cpuWorldRenderNodeFlag & (uint32_t)NodeDirtyFlags::MeshDirty)
        {
            OnRenderNodeMeshDirty(renderID, cpuWorldRenderNodePayload);
        }
        
        if((uint32_t)cpuWorldRenderNodeFlag & (uint32_t)NodeDirtyFlags::MaterialDirty)
        {
            OnRenderNodeMaterialDirty(renderID, cpuWorldRenderNodePayload);
        }
    }

    void CPUScene::EndFrame()
    {
    }

    void CPUScene::EnsureCapacity(uint32_t renderID)
    {
        int count = materialList.size();
        int need = renderID + 1;
        if(count < need)
        {
            materialList.resize(need);
            meshList.resize(need);
            worldBoundsList.resize(need);
            localBoundCacheList.resize(need);
            objectToWorldMatrixList.resize(need);
            layerList.resize(need, 1);
        }
    }
}
