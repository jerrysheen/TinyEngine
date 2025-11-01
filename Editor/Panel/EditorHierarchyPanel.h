#pragma once
#include "EditorPanel.h"

namespace EngineEditor
{
    class EditorHierarchyPanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorHierarchyPanel() override;        
    }; 

}