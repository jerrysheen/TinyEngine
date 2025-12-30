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
        uint32_t firstIndex = 0;
        uint32_t vertexOffset = 0;
        uint32_t indexInDrawIndirectList = 0;
        DrawIndirectParam(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset)
            : indexCount(indexCount), firstIndex(firstIndex), vertexOffset(vertexOffset)
        {

        }

        DrawIndirectParam() = default;
    };

    struct DrawIndirectContext
    {
        Material* material;
        uint32_t vaoID;
        DrawIndirectContext() = default;
        DrawIndirectContext(Material* mat, uint32_t vaoID): material(mat), vaoID(vaoID){}
    };

    class MeshRenderer;
    class BatchManager
    {
    public:
        static BatchManager* GetInstance()
        {
            if(s_Instance == nullptr)
            {
                Create();
            }
            return s_Instance;
        }

        void TryAddBatchCount(MeshRenderer* meshRenderer);
        void TryDecreaseBatchCount(MeshRenderer* meshRenderer);

        void TryAddBatchCount(MeshFilter* meshFilter);
        void TryDecreaseBatchCount(MeshFilter* meshFilter);
        static std::unordered_map<uint64_t, int> BatchMap;
        static std::unordered_map<uint64_t, DrawIndirectParam> drawIndirectParamMap;
        static std::unordered_map<uint64_t, DrawIndirectContext> drawIndirectContextMap;

        std::vector<RenderProxy> GetAvaliableRenderProxyList(MeshRenderer* meshRenderer, uint32_t vaoID); 
        static uint64_t GetBatchHash(MeshRenderer* meshRenderer, MeshFilter* meshFilter, uint32_t layer); 
        static uint64_t GetBatchHash(MeshRenderer* meshRenderer, uint32_t vaoID, uint32_t layer); 
        vector<DrawIndirectArgs> GetBatchInfo();
    private:
        void TryAddBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        void TryDecreaseBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        static void Create();
        static BatchManager* s_Instance;

    };
}