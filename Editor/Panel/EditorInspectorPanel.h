#pragma once
#include "EditorPanel.h"

namespace EngineEditor
{
    class EditorInspectorPanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorInspectorPanel() override;
    };

}