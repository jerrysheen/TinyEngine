#pragma once
#include "PreCompiledHeader.h"
#include "GameObject/Camera.h"
#include "Core/PublicStruct.h"
#include "Graphics/IGPUResource.h"

namespace EngineCore
{
    // 记录灯光，阴影，物体信息？
    class RenderContext
    {
    public:
        RenderContext()
        {
            visibleItems.resize(10000);
        }
        Camera* camera;
        vector<LightData*> visibleLights;
        vector<RenderPacket> visibleItems;
        
        inline void Reset()
        {
            camera = nullptr;
            visibleLights.clear();
            visibleItems.clear();
        }

        static void DrawRenderers(RenderContext& renderContext, 
                           const ContextDrawSettings& drawingSettings, 
                           const ContextFilterSettings& filteringSettings, 
                           std::vector<RenderBatch>& outDrawRecords);
        
        static void BatchContext(std::vector<RenderPacket*>& sortedItem);

        static bool CanBatch(const RenderBatch& batch, const RenderPacket item);

        IGPUBuffer* IndirectDrawArgsBuffer = nullptr;
        IGPUBuffer* CullingParamBuffer = nullptr;
        uint32_t currFrameIndex = 0;
    private:

    };
} // namespace EngineCore


