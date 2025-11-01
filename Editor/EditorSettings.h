#pragma once
#include "iostream"
#include "Math/Vector2.h"


namespace EngineEditor
{
using Vector2 = EngineCore::Vector2;
    // Editor
    class EditorSettings
    {
    private:
        // StartPos 和 Size都是0~1的ratio
        static Vector2  hierarchyStartPos;
        static Vector2  hierarchySize;
        static Vector2  consoleStartPos;
        static Vector2  consoleSize;
        static Vector2  projectStartPos;
        static Vector2  projectSize;
        static Vector2  inspectorStartPos;
        static Vector2  inspectorSize;
        static Vector2  mainBarStartPos;
        static Vector2  mainBarSize;
        static Vector2  gameViewStartPos;
        static Vector2  gameViewSize;

        static Vector2 currentWindowSize;
    public:
        inline static Vector2 GetHierarchyPanelStartPos(){return hierarchyStartPos * currentWindowSize;};
        inline static Vector2 GetHierarchyPanelEndPos(){return (hierarchyStartPos + hierarchySize) * currentWindowSize; };
        inline static Vector2 GetHierarchyPanelSize(){return hierarchySize * currentWindowSize;};

        inline static Vector2 GetConsolePanelStartPos(){return consoleStartPos * currentWindowSize;};
        inline static Vector2 GetConsolePanelEndPos(){return (consoleStartPos + consoleSize) * currentWindowSize;};
        inline static Vector2 GetConsolePanelSize(){return consoleSize * currentWindowSize;};

        inline static Vector2 GetProjectPanelStartPos(){return projectStartPos * currentWindowSize;};
        inline static Vector2 GetProjectPanelEndPos(){return (projectStartPos + projectSize) * currentWindowSize;};
        inline static Vector2 GetProjectPanelSize(){return projectSize * currentWindowSize;};

        inline static Vector2 GetInspectorPanelStartPos(){return inspectorStartPos * currentWindowSize;};
        inline static Vector2 GetInspectorPanelEndPos(){return (inspectorStartPos + inspectorSize) * currentWindowSize;};
        inline static Vector2 GetInspectorPanelSize(){return inspectorSize * currentWindowSize;};

        inline static Vector2 GetMainBarPanelStartPos(){return mainBarStartPos * currentWindowSize;};
        inline static Vector2 GetMainBarPanelEndPos(){return (mainBarStartPos + mainBarSize) * currentWindowSize;};
        inline static Vector2 GetMainBarPanelSize(){return mainBarSize * currentWindowSize;};

        inline static Vector2 GetGameViewPanelStartPos(){return gameViewStartPos * currentWindowSize;};
        inline static Vector2 GetGameViewPanelEndPos(){return (gameViewStartPos + gameViewSize) * currentWindowSize;};
        inline static Vector2 GetGameViewPanelSize(){return gameViewSize * currentWindowSize;};
        
        static void UpdateLayout(){};

    };


}