#include "PreCompiledHeader.h"
#include "BatchManager.h"
#include "Utils/HashCombine.h"
#include "Graphics/Mesh.h"
#include "Settings/ProjectSettings.h"
#include "Resources/ResourceManager.h"
#include "Resources/Asset.h"
namespace EngineCore
{
    BatchManager* BatchManager::s_Instance = nullptr;
    std::unordered_map<uint64_t, int> BatchManager::BatchMap;
    std::unordered_map<uint64_t, DrawIndirectContext> BatchManager::drawIndirectContextMap;
    std::unordered_map<uint64_t, DrawIndirectParam> BatchManager::drawIndirectParamMap;

    std::vector<RenderProxy> BatchManager::GetAvaliableRenderProxyList(AssetID meshID, AssetID materialID, uint32_t layer)
    {
        if(!meshID.IsValid() || !materialID.IsValid()) return std::vector<RenderProxy>();

        vector<RenderProxy> renderProxyList;
        uint32_t mask = layer;
        for(int i = 0 ; i < 32; i++)
        {
            if ((mask & (1u<<i)) != 0)
            {
                uint64_t hash = GetBatchHash(meshID, materialID, i);
                RenderProxy proxy;
                proxy.batchID = drawIndirectParamMap[hash].indexInDrawIndirectList;
                renderProxyList.push_back(proxy);
            }
        }

        return renderProxyList;
    }

    uint64_t BatchManager::GetBatchHash(AssetID meshID, AssetID materialID, uint32_t layer)
    {
        if(!meshID.IsValid() || !materialID.IsValid()) return 0;
        Mesh* mesh = ResourceManager::GetInstance()->GetResource<Mesh>(meshID);
        Material* material = ResourceManager::GetInstance()->GetResource<Material>(materialID);
        uint64_t batchKey = 0;
        uint32_t matKey = material->mRenderState.GetHash();
        HashCombine(matKey, static_cast<uint32_t>(layer));
        // 这个相当于VAOID
        uint32_t vaoID = mesh->GetInstanceID();
        uint32_t meshKey = vaoID;
        batchKey |= matKey;
        batchKey |= (static_cast<uint64_t>(meshKey) << 32);

        if(drawIndirectContextMap.count(batchKey) == 0)
        {
            drawIndirectContextMap[batchKey] = {material, mesh};
        }
        return batchKey; 
    }

    vector<DrawIndirectArgs> BatchManager::GetBatchInfo()
    {
        vector<DrawIndirectArgs> drawIndirectArgsList;
        int globalOffset = 0;
    
        // 【修改建议】: 将 unordered_map 转为 vector 并排序，确保遍历顺序稳定
        // BatchMap 的 key 是 hash (uint64_t)，排序后顺序是固定的
        std::vector<std::pair<uint64_t, int>> sortedBatches(BatchMap.begin(), BatchMap.end());
        std::sort(sortedBatches.begin(), sortedBatches.end());
    
        // 使用排序后的列表进行遍历
        for(auto& [key, value] : sortedBatches)
        {
            if(value <= 0) continue;
            DrawIndirectArgs args;
            ASSERT(drawIndirectParamMap.count(key) > 0);
            DrawIndirectParam& argsParam = drawIndirectParamMap[key];
            args.IndexCountPerInstanc = argsParam.indexCount;
            args.StartInstanceLocation = globalOffset; // 这个依旧需要设置， 不然GPU Culling会错。
            args.InstanceCount = 0;
            args.StartIndexLocation = 0;
            args.BaseVertexLocation = 0;
            if(RenderSettings::s_EnableVertexPulling)
            {
                args.StartIndexLocation = argsParam.startIndexLocation;
                args.BaseVertexLocation = argsParam.baseVertexLocation;
            }
            drawIndirectArgsList.push_back(args);
            
            // 更新 BatchID
            argsParam.startIndexInInstanceDataList = globalOffset;
            argsParam.indexInDrawIndirectList = drawIndirectArgsList.size() - 1;
            globalOffset += value;
        }   
        return drawIndirectArgsList;
    }

    void BatchManager::TryAddBatches(AssetID meshID, AssetID materialID, uint32_t layer)
    {
        if(!meshID.IsValid() || !materialID.IsValid()) return;
        Mesh* mesh = ResourceManager::GetInstance()->GetResource<Mesh>(meshID);
        Material* material = ResourceManager::GetInstance()->GetResource<Material>(materialID);
        uint32_t mask = layer;
        for(int i = 0 ; i < 32; i++)
        {
            if ((mask & (1u<<i)) != 0)
            {
                uint64_t batchKey = GetBatchHash(meshID, materialID, i);
                if(BatchMap.count(batchKey) > 0)
                {
                    BatchMap[batchKey] = BatchMap[batchKey] +1;;
                }
                else
                {
                    BatchMap[batchKey] = 1;
                    if(!RenderSettings::s_EnableVertexPulling)
                    {
                        drawIndirectParamMap[batchKey] = 
                        {
                            (uint32_t)mesh->indexAllocation->size / (uint32_t)sizeof(uint32_t), // 比如这个Mesh有300个索引
                            0,
                            0
                        };
                    }
                    else
                    {
                        drawIndirectParamMap[batchKey] = 
                        {
                            (uint32_t)mesh->indexAllocation->size / (uint32_t)sizeof(uint32_t), // 比如这个Mesh有300个索引
                            (uint32_t)mesh->indexAllocation->offset, 
                            (uint32_t)mesh->vertexAllocation->offset,
                        };
                    }
                }
            }
        }

    }

    void BatchManager::TryDecreaseBatches(AssetID meshID, AssetID materialID, uint32_t layer)
    {
        uint32_t mask = layer;
        for(int i = 0 ; i < 32; i++)
        {
            if ((mask & (1u<<i)) != 0)
            {
                uint64_t batchKey = GetBatchHash(meshID, materialID, i);
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