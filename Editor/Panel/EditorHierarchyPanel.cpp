#include "PreCompiledHeader.h"
#include "EditorHierarchyPanel.h"
#include "imgui.h"

namespace EngineEditor
{
    void EditorHierarchyPanel::DrawGUI()
    {
        bool alwaystrue = true;
        ImGui::Begin("Another Window", &alwaystrue);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                alwaystrue = false;
        ImGui::End();
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(1920, 1080);

    }

    EditorHierarchyPanel::~EditorHierarchyPanel()
    {
    }
}