#pragma once
#include <iostream>
#include "GameObject/Transform.h"
#include "Resources/Asset.h"
#include "Renderer/RenderUniforms.h"
#include "Math/Math.h"

namespace EngineCore
{

    enum class NodeDirtyFlags : uint32_t
    {
        None = 0,
        Created = 1 << 0,
        Destory = 1 << 1,
        TransformDirty = 1 << 2,
        MeshDirty = 1 << 4,
        MaterialDirty = 1 << 5,
    };

    struct NodeDirtyPayload
    {
        Transform* transform = nullptr;
        AssetID meshID = {};
        AssetID materialID = {};
        NodeDirtyPayload(Transform* trans, AssetID meshID, AssetID matID)
            : transform(trans), meshID(meshID), materialID(matID)
            {}
        NodeDirtyPayload() = default;
        NodeDirtyPayload(Transform* trans)
            : transform(trans){}
    };

    struct CPUSceneView
    {
        const std::vector<Matrix4x4>& objectToWorldList;
        const std::vector<AABB>& worldBoundsList;
        const std::vector<AssetID>& materialList;
        const std::vector<AssetID>& meshList;
        const std::vector<uint32_t>& layerList;
        CPUSceneView() = default;

        CPUSceneView(
            const std::vector<Matrix4x4>& objectToWorld,
            const std::vector<AABB>& worldBounds,
            const std::vector<AssetID>& materials,
            const std::vector<AssetID>& meshes,
            const std::vector<uint32_t>& layers)
            : objectToWorldList(objectToWorld)
            , worldBoundsList(worldBounds)
            , materialList(materials)
            , meshList(meshes)
            , layerList(layers)
        {}
    };

    struct GPUSceneDelta
    {
        vector<uint32_t> dirtyRenderNodeIDList;
        vector<uint32_t> dirtyRenderNodeFlagsList;
    };
    
}