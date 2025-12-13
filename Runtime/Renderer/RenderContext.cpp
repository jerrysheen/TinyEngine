#include "PreCompiledHeader.h"
#include "RenderContext.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "Core/Profiler.h"
#include "RenderSorter.h"
#include "Graphics/GPUSceneManager.h"

namespace EngineCore
{
    // move 进来的时候直接在Culling中显式std::move
    VisibleItem* RenderContext::GetAvalileVisibleItem()
    {        
        if(visibleItemPool.size() > 0)
        {
            auto res =  visibleItemPool.back();
            visibleItemPool.pop_back();
            return res;
        }
        return new VisibleItem();
    }

    LightData* RenderContext::GetAvalibleLightData()
    {        
        if(lightDataPool.size() > 0)
        {
            auto res =  lightDataPool.back();
            lightDataPool.pop_back();
            return res;
        }
        return new LightData();
    }

    void RenderContext::DrawRenderers(const RenderContext &renderContext, const ContextDrawSettings &drawingSettings, const ContextFilterSettings &filteringSettings, std::vector<RenderBatch> & bactchList)
    {
        vector<SortItem> sortItems;

        PROFILER_EVENT_BEGIN("DrawRenderers::SortingContext");
        RendererSort::BuildSortKeys(renderContext, renderContext.cameraVisibleItems, sortItems, drawingSettings.sortingCriteria);
        PROFILER_EVENT_END("DrawRenderers::SortingContext");

        PROFILER_EVENT_BEGIN("DrawRenderers::SortingContext");
        std::sort(sortItems.begin(), sortItems.end(), [](SortItem& a, SortItem& b) { return a.sortKey < b.sortKey; });
        PROFILER_EVENT_END("DrawRenderers::SortingContext");
        
        
        vector<uint32_t> tempList;
        if(sortItems.size() == 0) return;
        auto& items = renderContext.cameraVisibleItems[sortItems[0].itemIndex];
        RenderBatch currentBatch;
        currentBatch.instanceCount = 1;
        currentBatch.model = items->meshFilter->mMeshHandle.Get();
        currentBatch.mat = items->meshRenderer->GetMaterial().Get();
        currentBatch.index = 0;
        uint32_t objIndex = items->meshRenderer->perObjectDataAllocation.offset / items->meshRenderer->perObjectDataAllocation.size;
        tempList.push_back(objIndex);
        uint32_t globalIndex = 0;
        for(int i = 1; i < sortItems.size(); i++)
        {
            auto& items = renderContext.cameraVisibleItems[sortItems[i].itemIndex];
            if(CanBatch(currentBatch, items))
            {
                currentBatch.instanceCount += 1;
                objIndex = items->meshRenderer->perObjectDataAllocation.offset / items->meshRenderer->perObjectDataAllocation.size;
                tempList.push_back(objIndex);
                globalIndex += 1;
            }
            else
            {
                currentBatch.alloc =  GPUSceneManager::GetInstance()->SyncDataToPerFrameBatchBuffer(tempList.data(), tempList.size());
                bactchList.push_back(currentBatch);
                
                tempList.clear();
                currentBatch.instanceCount = 1;
                currentBatch.model = items->meshFilter->mMeshHandle.Get();
                currentBatch.mat = items->meshRenderer->GetMaterial().Get();
                currentBatch.index = globalIndex;
                uint32_t objIndex = items->meshRenderer->perObjectDataAllocation.offset / items->meshRenderer->perObjectDataAllocation.size;
                tempList.push_back(objIndex);
                globalIndex++;
            }
        }
        if (tempList.size() > 0) 
        {
            currentBatch.alloc = GPUSceneManager::GetInstance()->SyncDataToPerFrameBatchBuffer(tempList.data(), tempList.size());
            bactchList.push_back(currentBatch);
            tempList.clear();
        }


    }

    void RenderContext::SortingContext(const RenderContext &renderContext, const ContextDrawSettings &drawingSettings, std::vector<VisibleItem*> &sortedItem)
    {
        switch(drawingSettings.sortingCriteria)
        {
            case SortingCriteria::ComonOpaque:
                for(auto& item : sortedItem)
                {
                    Vector3 diff = renderContext.camera->gameObject->transform->GetWorldPosition() - item->transform->GetWorldPosition();
                    item->distanToCamera = Vector3::Dot(diff, diff);
                }

                std::sort(sortedItem.begin(), sortedItem.end(), [&](VisibleItem* itemA, VisibleItem* itemB)
                {
                    // true排在队列前面，相当于先渲染。
                    Material* matA = itemA->meshRenderer->GetSharedMaterial();
                    Material* matB = itemB->meshRenderer->GetSharedMaterial();
                    if(matA != matB) return matA < matB;
                    ModelData* modelA = itemA->meshFilter->mMeshHandle.Get();
                    ModelData* modelB = itemB->meshFilter->mMeshHandle.Get();
                    if(modelA != modelB) return modelA < modelB;
                    return itemA->distanToCamera < itemB->distanToCamera;
                });
            break;
            default:
            break;
        }
    }


    void RenderContext::BatchContext(std::vector<VisibleItem *> &sortedItem)
    {
    }

    bool RenderContext::CanBatch(const RenderBatch& currentBatch, const VisibleItem* item)
    {
        return currentBatch.model->GetAssetID() == item->meshFilter->mMeshHandle->GetAssetID() &&
            currentBatch.mat->GetAssetID() == item->meshRenderer->GetMaterial()->GetAssetID();
    }

    void RenderContext::ReturnLightToPool()
    {
        lightDataPool.insert(
            lightDataPool.end(),
            std::make_move_iterator(visibleLights.begin()),
            std::make_move_iterator(visibleLights.end()));
        visibleLights.clear();
    }

    void RenderContext::ReturnItemToPool()
    {
        visibleItemPool.insert(
            visibleItemPool.end(),
            cameraVisibleItems.begin(),
            cameraVisibleItems.end());
        cameraVisibleItems.clear();
    }

} // namespace EngineCore