#pragma once
#include <iostream>
#include <cmath>
#include <vector>
#include "Core/PublicStruct.h"
#include "RenderContext.h"
#include "Math/Math.h"

namespace EngineCore
{

    inline uint16_t FloatToDepth(float depth, float nearPlane, float farPlane)
    {
        float normalized = (depth - nearPlane) / (farPlane - nearPlane);
        normalized = std::max(0.0f, std::min(1.0f, normalized));

        return static_cast<uint16_t>(normalized * 65535.0f);
    }    

    class RendererSort
    {
    public:
        static void BuildSortKeys(const RenderContext& context, std::vector<RenderPacket>& items,
                                SortingCriteria sortingCriteria)
        {
            Vector3 camPos = context.camera->gameObject->transform->GetWorldPosition();
            float nearPlane = context.camera->mNear;
            float farPlane = context.camera->mFar;
            for(auto& item : items)
            {
                uint32_t meshID = item.meshFilter->mMeshHandle->GetAssetID();
                uint32_t matID = item.meshRenderer->GetMaterial()->GetAssetID();

                float distance = item.distanToCamera;
                uint16_t distanceToID = FloatToDepth(distance, nearPlane, farPlane);
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
                item.sortingKey = key;
            }
        }
    };

};