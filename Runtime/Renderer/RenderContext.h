#pragma once
#include "PreCompiledHeader.h"
#include "Graphics/Camera.h"
#include "Core/PublicStruct.h"

namespace EngineCore
{
    // 记录灯光，阴影，物体信息？
    class RenderContext
    {
    public:
        Camera* camera;
        vector<LightData*> visibleLights;
        vector<Matrix4x4> shadowMatrixs;
        vector<VisibleItem*> cameraVisibleItems;
        vector<vector<VisibleItem*>> shadowsVisibleItems;
    
        inline void Reset()
        {
            camera = nullptr;
            ReturnLightToPool();
            ReturnItemToPool();
            shadowMatrixs.clear();
        }

        VisibleItem* GetAvalileVisibleItem();
        LightData* GetAvalibleLightData();
    private:
        void ReturnLightToPool();
        void ReturnItemToPool();
    private: 
        vector<VisibleItem*> visibleItemPool;
        vector<LightData*> lightDataPool;

    };
} // namespace EngineCore

