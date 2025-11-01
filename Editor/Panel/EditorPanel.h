#pragma once

namespace EngineEditor
{
    class EditorPanel
    {
    public: 
        virtual void DrawGUI() = 0;
        virtual ~EditorPanel() = 0;
    };

}