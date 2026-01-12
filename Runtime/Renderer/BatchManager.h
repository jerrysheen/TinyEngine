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
        uint32_t startIndexInInstanceDataList = 0;
        uint32_t vertexOffset = 0;
        uint32_t indexInDrawIndirectList = 0;
        DrawIndirectParam(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset)
            : indexCount(indexCount), startIndexInInstanceDataList(firstIndex), vertexOffset(vertexOffset)
        {

        }

        DrawIndirectParam() = default;
    };

    struct DrawIndirectContext
    {
        Material* material;
        Mesh* mesh;
        DrawIndirectContext() = default;
        DrawIndirectContext(Material* mat, Mesh* mesh): material(mat), mesh(mesh){}
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

        std::vector<RenderProxy> GetAvaliableRenderProxyList(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        static uint64_t GetBatchHash(MeshRenderer* meshRenderer, MeshFilter* meshFilter, uint32_t layer); 

        vector<DrawIndirectArgs> GetBatchInfo();
    private:
        void TryAddBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        void TryDecreaseBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        static void Create();
        static BatchManager* s_Instance;

    };
}