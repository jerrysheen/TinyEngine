#include "PreCompiledHeader.h"
#include "EditorInspectorPanel.h"
#include "imgui.h"
#include "EditorSettings.h"
#include "EditorGUIManager.h"

namespace EngineEditor
{
    std::unordered_map<EngineCore::Transform*, EditorInspectorPanel::EulerUICache>
        EditorInspectorPanel::sEulerCache;

    using Transform = EngineCore::Transform;
    // Inspector 透明度设置为0.0， 只用来接受以及处理一些SceneView事件
    void EditorInspectorPanel::DrawGUI()
    {
        ImGui::SetNextWindowPos(ImVec2((float)EditorSettings::GetInspectorPanelStartPos().x, (float)EditorSettings::GetInspectorPanelStartPos().y));
        ImGui::SetNextWindowSize(ImVec2((float)EditorSettings::GetInspectorPanelSize().x, (float)EditorSettings::GetInspectorPanelSize().y));
        
        if (ImGui::Begin("Inspector", NULL, 
            ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoResize))
        {
            auto* currentSelected = EditorGUIManager::GetInstance().GetCurrentSelected();
            if(currentSelected != nullptr)
            {
                auto* transform = currentSelected->GetComponent<Transform>();
                DrawTransformComponent(transform);
            }

        }
        ImGui::End();
    }
    
    void EditorInspectorPanel::DrawTransformComponent(EngineCore::Transform* transform)
    {
        if (transform == nullptr) return;
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (!ImGui::CollapsingHeader("Transform"))
            return;

        ImGui::PushItemWidth(60);

        // --- Position（保持你原来的逻辑：这是局部位移；若你想“世界位移”，可改成TranslateWorld）---
        Vector3 position = transform->GetLocalPosition();
        ImGui::Text("Position    ");
        ImGui::SameLine(); ImGui::Text("X");
        ImGui::SameLine(); bool posXChanged = ImGui::DragFloat("##posX", &position.x, 0.15f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Y");
        ImGui::SameLine(); bool posYChanged = ImGui::DragFloat("##posY", &position.y, 0.15f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Z");
        ImGui::SameLine(); bool posZChanged = ImGui::DragFloat("##posZ", &position.z, 0.15f, -FLT_MAX, FLT_MAX);
        if (posXChanged || posYChanged || posZChanged) {
            transform->SetLocalPosition(position);
        }

        // --- Rotation（改为“增量四元数 + 世界轴左乘”，UI 用缓存）---
        // 1) 取/初始化缓存（仅第一次对这个Transform显示时，从真实姿态做一次seed）
        auto& cache = sEulerCache[transform];
        if (!cache.seeded) {
            cache.eulerDeg = transform->GetLocalEulerAngles(); // 仅一次；之后不再反解
            cache.seeded = true;
        }

        Vector3 uiEuler = cache.eulerDeg; // 用缓存作为当前显示值
        ImGui::Text("Rotation    ");
        ImGui::SameLine(); ImGui::Text("X");
        ImGui::SameLine(); bool rotXChanged = ImGui::DragFloat("##rotX", &uiEuler.x, 0.25f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Y");
        ImGui::SameLine(); bool rotYChanged = ImGui::DragFloat("##rotY", &uiEuler.y, 0.25f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Z");
        ImGui::SameLine(); bool rotZChanged = ImGui::DragFloat("##rotZ", &uiEuler.z, 0.25f, -FLT_MAX, FLT_MAX);

        if (rotXChanged || rotYChanged || rotZChanged) {
            // 2) 计算“本次UI操作”的增量角（对每根世界轴相互独立）
            Vector3 d;
            d.x = WrapDelta180(uiEuler.x - cache.eulerDeg.x);
            d.y = WrapDelta180(uiEuler.y - cache.eulerDeg.y);
            d.z = WrapDelta180(uiEuler.z - cache.eulerDeg.z);

            // 3) 世界轴左乘：谁变了应用谁（顺序不重要，因为是三根独立世界轴的微小增量；
            //    如果你希望严格模仿Unity，可按 X→Y→Z 顺序也行）
            if (std::abs(d.x) > 0.0001f) transform->RotateX(d.x);
            if (std::abs(d.y) > 0.0001f) transform->RotateY(d.y);
            if (std::abs(d.z) > 0.0001f) transform->RotateZ(d.z);

            // 4) 更新UI缓存（显示值直接用缓存，不要从四元数反解）
            cache.eulerDeg = uiEuler;
        }

        // --- Scale（保持原样）---
        Vector3 scale = transform->GetLocalScale();
        ImGui::Text("Scale       ");
        ImGui::SameLine(); ImGui::Text("X");
        ImGui::SameLine(); bool scaXChanged = ImGui::DragFloat("##scaX", &scale.x, 0.01f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Y");
        ImGui::SameLine(); bool scaYChanged = ImGui::DragFloat("##scaY", &scale.y, 0.01f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine(); ImGui::Text("Z");
        ImGui::SameLine(); bool scaZChanged = ImGui::DragFloat("##scaZ", &scale.z, 0.01f, -FLT_MAX, FLT_MAX);
        if (scaXChanged || scaYChanged || scaZChanged) {
            transform->SetLocalScale(scale);
        }

        ImGui::PopItemWidth();
    }

    EditorInspectorPanel::~EditorInspectorPanel()
    {
    }
}