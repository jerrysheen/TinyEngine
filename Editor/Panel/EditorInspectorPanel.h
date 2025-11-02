#pragma once
#include "EditorPanel.h"
#include "GameObject/Transform.h"

namespace EngineEditor
{
    class EditorInspectorPanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorInspectorPanel() override;
        void DrawTransformComponent(EngineCore::Transform* transform);

        // 一个小工具：把角度差wrap到[-180,180]，避免359→-1的大跳
        static inline float WrapDelta180(float d) {
            while (d > 180.f) d -= 360.f;
            while (d < -180.f) d += 360.f;
            return d;
        }

        // 给每个Transform维护一份UI角度缓存（仅用于显示/差分计算）
        struct EulerUICache 
        { 
            EulerUICache() = default;
            Vector3 eulerDeg = { 0,0,0 }; 
            bool seeded = false; 
        };

        static std::unordered_map<EngineCore::Transform*, EulerUICache> sEulerCache;
    };

}