#pragma once
#include <vector>
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "Math/Math.h"
#include "Scene/Scene.h"
#include "SceneStruct.h"

using std::vector;
namespace EngineCore
{
    class CPUScene
    {
    public:
        void Update(uint32_t frameID);
      
        void ApplyDirtyNode(uint32_t renderID, NodeDirtyFlags cpuWorldRenderNodeFlag , NodeDirtyPayload& payload);
        void EndFrame();
        CPUSceneView GetSceneView();

        inline void SetCurrentFrame(uint32_t currentFrame)
        {   
            mCurrentFrame = currentFrame;
        }
    private:
        void EnsureCapacity(uint32_t renderID);
        void CreateRenderNode(uint32_t renderID, NodeDirtyPayload& payload);
        void DeleteRenderNode(uint32_t renderID); 
        void OnRenderNodeMaterialDirty(uint32_t renderID, NodeDirtyPayload& payload);
        void OnRenderNodeTransformDirty(uint32_t renderID, NodeDirtyPayload& payload);
        void OnRenderNodeMeshDirty(uint32_t renderID, NodeDirtyPayload& payload);
    
        
    private:
        vector<AssetID> materialList;
        vector<AssetID> meshList;
        vector<AABB> worldBoundsList;
        vector<AABB> localBoundCacheList;
        vector<Matrix4x4> objectToWorldMatrixList;
        vector<uint32_t> layerList;

        uint32_t mCurrentFrame = 0;
    };
}