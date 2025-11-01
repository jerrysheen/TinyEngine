#pragma once
#include "EditorPanel.h"

namespace EngineEditor
{
    class EditorProjectPanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorProjectPanel() override;
    };

}