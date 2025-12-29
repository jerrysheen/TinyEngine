#include "PreCompiledHeader.h"
#include "BatchManager.h"
#include "Utils/HashCombine.h"
#include "Graphics/ModelData.h"

namespace EngineCore
{
    BatchManager* BatchManager::s_Instance = nullptr;

    void BatchManager::TryAddBatchCount(MeshRenderer *meshRenderer)
    {
        MeshFilter* meshFilter = meshRenderer->gameObject->GetComponent<MeshFilter>();
        if(meshFilter == nullptr) return;
        // todo :  这个地方的生成逻辑会麻烦一点，不单单是通过meshRenderer去创建。
        // 一个meshRenderer会延伸出多个 renderProxy
        TryAddBatches(meshRenderer, meshFilter);
    }

    void BatchManager::TryDecreaseBatchCount(MeshRenderer *meshRenderer)
    {
        MeshFilter* meshFilter = meshRenderer->gameObject->GetComponent<MeshFilter>();
        if(meshFilter == nullptr) return;
        TryDecreaseBatches(meshRenderer, meshFilter);

    }

    void BatchManager::TryAddBatchCount(MeshFilter * meshFilter)
    {
        MeshRenderer* meshRenderer = meshFilter->gameObject->GetComponent<MeshRenderer>();
        if(meshRenderer == nullptr) return;
        TryAddBatches(meshRenderer, meshFilter);
    }

    void BatchManager::TryDecreaseBatchCount(MeshFilter * meshFilter)
    {
        MeshRenderer* meshRenderer = meshFilter->gameObject->GetComponent<MeshRenderer>();
        if(meshRenderer == nullptr) return;
        TryDecreaseBatches(meshRenderer, meshFilter);

    }

    std::vector<RenderProxy> BatchManager::GetAvaliableRenderProxyList(MeshRenderer *meshRenderer, uint32_t vaoID)
    {
        ASSERT(meshRenderer != nullptr);
        vector<RenderProxy> renderProxyList;
        uint32_t mask = meshRenderer->renderLayer;
        for(int i = 0 ; i < 32; i++)
        {
            if ((mask & (1u<<i)) != 0)
            {
                uint64_t hash = GetBatchHash(meshRenderer, vaoID, i);
                RenderProxy proxy;
                proxy.batchID = drawIndirectParamMap[hash].indexInDrawIndirectList;
                renderProxyList.push_back(proxy);
            }
        }

        return renderProxyList;
    }

    uint64_t BatchManager::GetBatchHash(MeshRenderer *meshRenderer, MeshFilter *meshFilter, uint32_t layer)
    {
        uint64_t batchKey = 0;
        uint32_t matKey = meshRenderer->GetMaterial()->mRenderState.GetHash();
        HashCombine(matKey, static_cast<uint32_t>(layer));
        // 这个相当于VAOID
        uint32_t vaoID = meshFilter->mMeshHandle.Get()->GetInstanceID();
        uint32_t meshKey = vaoID;
        batchKey |= matKey;
        batchKey |= (static_cast<uint64_t>(meshKey) << 32);
        return batchKey; 
    }

    uint64_t BatchManager::GetBatchHash(MeshRenderer *meshRenderer, uint32_t vaoID, uint32_t layer)
    {
        uint64_t batchKey = 0;
        uint32_t matKey = meshRenderer->GetMaterial()->mRenderState.GetHash();
        HashCombine(matKey, static_cast<uint32_t>(layer));
        uint32_t meshKey = vaoID;
        batchKey |= matKey;
        batchKey |= (static_cast<uint64_t>(meshKey) << 32);
        return batchKey; 
    }

    vector<DrawIndirectArgs> BatchManager::GetBatchInfo()
    {
        vector<DrawIndirectArgs> drawIndirectArgsList;
        int globalOffset = 0;
        for(auto& [key, value] : BatchMap)
        {
            if(value <= 0) continue;
            DrawIndirectArgs args;
            ASSERT(drawIndirectParamMap.count(key) > 0);
            DrawIndirectParam& argsParam = drawIndirectParamMap[key];
            args.indexCount = argsParam.indexCount;
            args.firstInstance = globalOffset;
            drawIndirectArgsList.push_back(args);
            argsParam.indexInDrawIndirectList = drawIndirectArgsList.size() - 1;
            globalOffset += value;
        }   
        return vector<DrawIndirectArgs>();
    }

    void BatchManager::TryAddBatches(MeshRenderer *meshRenderer, MeshFilter *meshFilter)
    {
        uint32_t mask = meshRenderer->renderLayer;
        for(int i = 0 ; i < 32; i++)
        {
            if ((mask & (1u<<i)) != 0)
            {
                uint64_t batchKey = GetBatchHash(meshRenderer, meshFilter, i);
                if(BatchMap.count(batchKey) > 0)
                {
                    BatchMap[batchKey] = BatchMap[batchKey] +1;;
                }
                else
                {
                    BatchMap[batchKey] = 1;
                    ModelData* modeldata = meshFilter->mMeshHandle.Get();
                    ASSERT(modeldata != nullptr);
                    drawIndirectParamMap[batchKey] = 
                    {
                        (uint32_t)modeldata->index.size(), // 比如这个Mesh有300个索引
                        0,
                        0
                    };
                }
            }
        }

    }

    void BatchManager::TryDecreaseBatches(MeshRenderer *meshRenderer, MeshFilter *meshFilter)
    {
        uint32_t mask = meshRenderer->renderLayer;
        for(int i = 0 ; i < 32; i++)
        {
            if ((mask & (1u<<i)) != 0)
            {
                uint64_t batchKey = GetBatchHash(meshRenderer, meshFilter, i);
                BatchMap[batchKey] = BatchMap[batchKey] - 1;
            }
        }

    }

    void BatchManager::Create()
    {
        if(s_Instance == nullptr)
        {
            s_Instance = new BatchManager();
        }
    }
}