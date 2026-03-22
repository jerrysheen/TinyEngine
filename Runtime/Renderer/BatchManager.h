#pragma once
#include "Renderer/RenderStruct.h"
#include "Graphics/GPUBufferAllocator.h"
#include <unordered_map>
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "Renderer/RenderStruct.h"

namespace EngineCore
{
    struct DrawIndirectParam
    {
        uint32_t indexCount = 0; // 比如这个Mesh有300个索引
        uint32_t startIndexInInstanceDataList = 0; // visiblityBuffer中的index
        uint32_t indexInDrawIndirectList = 0; //indirectDrawCallBuffer中的index
        uint32_t startIndexLocation;    // mesh big buffer 中的index index
        uint32_t baseVertexLocation;    // mesh big buffer 中的vertex index；
        DrawIndirectParam(uint32_t indexCount, uint32_t startIndex, uint32_t baseVertex)
            : indexCount(indexCount), startIndexLocation(startIndex), baseVertexLocation(baseVertex)
        {

        }

        DrawIndirectParam() = default;
    };

    struct DrawIndirectContext
    {
        int batchIndex = -1;
        Material* material;
        Mesh* mesh;
        DrawIndirectContext() = default;
        DrawIndirectContext(int index, Material* mat, Mesh* mesh) : material(mat), mesh(mesh), batchIndex(index) {}
    };

    class MeshRenderer;
    class BatchManager
    {
    public:
        static BatchManager* GetInstance()
        {
            if (s_Instance == nullptr)
            {
                Create();
            }
            return s_Instance;
        }
        void TryAddBatches(AssetID meshID, AssetID materialID, uint32_t layer);
        void TryDecreaseBatches(AssetID meshID, AssetID materialID, uint32_t layer);
        std::unordered_map<uint64_t, DrawIndirectParam> drawIndirectParamMap;
        std::unordered_map<uint64_t, DrawIndirectContext> drawIndirectContextMap;
        std::unordered_map<uint64_t, int> BatchMap;
        // 记录Batch的顺序，保证每次GetBatchInfo得到的数据稳定， 只会添加
        std::vector<uint64_t> BatchList;

        std::vector<RenderProxy> GetAvaliableRenderProxyList(AssetID meshID, AssetID materialID, uint32_t layer);

        vector<DrawIndirectArgs> GetBatchInfo();

        inline const std::unordered_map<uint64_t, DrawIndirectContext>& GetDrawIndirectContextMap() const { return drawIndirectContextMap; }

        inline std::unordered_map<uint64_t, DrawIndirectParam>& GetDrawIndirectParamMap() { return drawIndirectParamMap; }
    private:
        uint64_t GetBatchHash(AssetID meshID, AssetID materialID, uint32_t layer);

        static void Create();
        static BatchManager* s_Instance;

    };
}