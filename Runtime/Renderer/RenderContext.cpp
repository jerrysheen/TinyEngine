#include "PreCompiledHeader.h"
#include "RenderContext.h"

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
        for (auto ptr : cameraVisibleItems) 
        {
            visibleItemPool.push_back(std::move(cameraVisibleItems.back()));
            cameraVisibleItems.pop_back();
        }
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