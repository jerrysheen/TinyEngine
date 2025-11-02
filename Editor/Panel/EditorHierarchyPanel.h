#pragma once
#include "EditorPanel.h"
#include "GameObject/GameObject.h"
#include "imgui.h"

namespace EngineEditor
{
    using EngineCore::GameObject;
    class EditorHierarchyPanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorHierarchyPanel() override;  
    private:
        int nodeIdx = 0;
        EngineCore::GameObject* selectedGO = nullptr;
        ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | 
            ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;   
        void DrawNode(GameObject* go);   
    }; 

}