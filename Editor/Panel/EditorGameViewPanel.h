#pragma once
#include "EditorPanel.h"

namespace EngineEditor
{
    class EditorGameViewPanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorGameViewPanel() override;
    };

}