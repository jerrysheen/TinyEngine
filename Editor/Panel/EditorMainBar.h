#pragma once
#include "EditorPanel.h"

namespace EngineEditor
{
    class EditorMainBar : public EditorPanel
    {
    public:
        virtual void DrawGUI() override;
        virtual ~EditorMainBar() override;
    };

}