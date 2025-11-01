#pragma once
#include "EditorPanel.h"

namespace EngineEditor
{
    class EditorConsolePanel : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorConsolePanel() override;
    };

}