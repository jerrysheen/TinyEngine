#include "PreCompiledHeader.h"
#include "RenderContext.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "Core/Profiler.h"
#include "RenderSorter.h"

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

    void RenderContext::DrawRenderers(const RenderContext &renderContext, const ContextDrawSettings &drawingSettings, const ContextFilterSettings &filteringSettings, std::vector<DrawRecord> &outDrawRecords)
    {
        vector<SortItem> sortItems;

        PROFILER_EVENT_BEGIN("DrawRenderers::SortingContext");
        RendererSort::BuildSortKeys(renderContext, renderContext.cameraVisibleItems, sortItems, drawingSettings.sortingCriteria);
        PROFILER_EVENT_END("DrawRenderers::SortingContext");

        PROFILER_EVENT_BEGIN("DrawRenderers::SortingContext");
        std::sort(sortItems.begin(), sortItems.end(), [](SortItem& a, SortItem& b) { return a.sortKey < b.sortKey; });
        PROFILER_EVENT_END("DrawRenderers::SortingContext");

        for(int i = 0; i < sortItems.size(); i++)
        {
            auto& items = renderContext.cameraVisibleItems[sortItems[i].itemIndex];
            // todo: 后续会换成不同的比如opaquepass独有的data
            auto& mpb = items->meshRenderer->GetMaterialPropertyBlock();
            mpb.SetValue("WorldMatrix", items->transform->GetWorldMatrix());

            PerDrawHandle handle = RenderAPI::GetInstance()->AllocatePerDrawData(mpb.GetSize());
            memcpy(handle.destPtr, mpb.GetData(), mpb.GetSize());
            outDrawRecords.emplace_back(items->meshRenderer->GetMaterial().Get(), items->meshFilter->mMeshHandle.Get(), handle, 1);
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
                    if(matA != matB) return *matA < *matB;
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
        //visibleItemPool.insert(
        //    visibleItemPool.end(),
        //    std::make_move_iterator(cameraVisibleItems.begin()),
        //    std::make_move_iterator(cameraVisibleItems.end()));
        //for(auto& i : shadowsVisibleItems)
        //{
        //    visibleItemPool.insert(
        //        visibleItemPool.end(),
        //        std::make_move_iterator(i.begin()),
        //        std::make_move_iterator(i.end()));
        //}
    }

} // namespace EngineCore