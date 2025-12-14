#include "PreCompiledHeader.h"
#include "RenderContext.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "Core/Profiler.h"
#include "RenderSorter.h"
#include "Graphics/GPUSceneManager.h"

namespace EngineCore
{


    void RenderContext::DrawRenderers(RenderContext &renderContext, const ContextDrawSettings &drawingSettings, const ContextFilterSettings &filteringSettings, std::vector<RenderBatch> & bactchList)
    {
        vector<RenderPacket>& visibleItems = renderContext.visibleItems;
        PROFILER_EVENT_BEGIN("DrawRenderers::SortingContext");
        RendererSort::BuildSortKeys(renderContext, renderContext.visibleItems, drawingSettings.sortingCriteria);
        PROFILER_EVENT_END("DrawRenderers::SortingContext");

        PROFILER_EVENT_BEGIN("DrawRenderers::SortingContext");
        std::sort(visibleItems.begin(), visibleItems.end(), [](RenderPacket& a, RenderPacket& b) { return a.sortingKey < b.sortingKey; });
        PROFILER_EVENT_END("DrawRenderers::SortingContext");
        
        
        vector<uint32_t> tempList;
        if(visibleItems.size() == 0) return;
        auto& item = visibleItems[0];
        RenderBatch currentBatch;
        currentBatch.instanceCount = 1;
        currentBatch.vaoID = item.vaoID;
        currentBatch.mat = item.meshRenderer->GetMaterial().Get();
        currentBatch.index = 0;
        // todo : 回头这个东西应该也是CPU端映射的。
        uint32_t objIndex = item.meshRenderer->perObjectDataAllocation.offset / item.meshRenderer->perObjectDataAllocation.size;
        tempList.push_back(objIndex);
        uint32_t globalIndex = 0;
        for(int i = 1; i < visibleItems.size(); i++)
        {
            item = visibleItems[i];
            if(CanBatch(currentBatch, item))
            {
                currentBatch.instanceCount += 1;
                objIndex = item.meshRenderer->perObjectDataAllocation.offset / item.meshRenderer->perObjectDataAllocation.size;
                tempList.push_back(objIndex);
                globalIndex += 1;
            }
            else
            {
                currentBatch.alloc =  GPUSceneManager::GetInstance()->SyncDataToPerFrameBatchBuffer(tempList.data(), tempList.size());
                bactchList.push_back(currentBatch);
                
                tempList.clear();
                currentBatch.instanceCount = 1;
                currentBatch.vaoID = item.vaoID;;
                currentBatch.mat = item.meshRenderer->GetMaterial().Get();
                currentBatch.index = globalIndex;
                uint32_t objIndex = item.meshRenderer->perObjectDataAllocation.offset / item.meshRenderer->perObjectDataAllocation.size;
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

    void RenderContext::BatchContext(std::vector<RenderPacket *> &sortedItem)
    {
    }

    bool RenderContext::CanBatch(const RenderBatch& currentBatch, const RenderPacket item)
    {
        return currentBatch.vaoID == item.vaoID &&
            currentBatch.mat->GetAssetID() == item.meshRenderer->GetMaterial()->GetAssetID();
    }
} // namespace EngineCore