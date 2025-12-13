#pragma once
#include <iostream>
#include <cmath>
#include <vector>
#include "Core/PublicStruct.h"
#include "RenderContext.h"

namespace EngineCore
{
    struct SortItem
    {
        uint64_t sortKey;
        int itemIndex;
        SortItem(uint64_t key, int index):sortKey(key), itemIndex(index){}
    };

    inline uint16_t FloatToDepth(float depth, float nearPlane, float farPlane)
    {
        float normalized = (depth - nearPlane) / (farPlane - nearPlane);
        normalized = std::max(0.0f, std::min(1.0f, normalized));

        return static_cast<uint16_t>(normalized * 65535.0f);
    }
    
    class RendererSort
    {
    public:
        static void BuildSortKeys(const RenderContext& context, const std::vector<VisibleItem*>& items,
                                std::vector<SortItem>& outSortItems,
                                SortingCriteria sortingCriteria)
        {
            outSortItems.clear();
            outSortItems.reserve(items.size());
            int index = 0;
            for(auto* item : items)
            {
                uint32_t meshID = item->meshFilter->mMeshHandle->GetAssetID();
                uint32_t matID = item->meshRenderer->GetMaterial()->GetAssetID();

                float distance = Vector3::Length(context.camera->gameObject->transform->GetWorldPosition() - item->transform->GetWorldPosition());
                uint16_t distanceToID = FloatToDepth(distance, context.camera->mNear, context.camera->mFar);
                switch(sortingCriteria)
                {
                    case SortingCriteria::ComonOpaque:
                    break;
                    case SortingCriteria::ComonTransparent:
                        distanceToID = 0xFFFF - distanceToID;
                    break;
                }
                // todo : 用游戏isntanceID，改成24位足够了
                uint64_t key = 0;
                key |= (uint64_t)(0) << 60; // Layer 0
                key |= (uint64_t)(matID & 0x0FFFFFFF) << 32; 
                key |= (uint64_t)(meshID & 0xFFFF) << 16;
                key |= (uint64_t)(distanceToID & 0xFFFF); 
                outSortItems.push_back({key, index});
                index++;
            }
        }
    };

};