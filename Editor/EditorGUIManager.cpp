#include "PreCompiledHeader.h"
#include "EditorGUIManager.h"
#include "D3D12/D3D12EditorGUIManager.h"
#include "Renderer/RenderEngine.h"
#include "Panel/EditorConsolePanel.h"
#include "Panel/EditorGameViewPanel.h"
#include "Panel/EditorHierarchyPanel.h"
#include "Panel/EditorInspectorPanel.h"
#include "Panel/EditorMainBar.h"
#include "Panel/EditorProjectPanel.h"

namespace EngineEditor
{
    EditorGUIManager* EditorGUIManager::s_Instance = nullptr;

    EditorGUIManager::~EditorGUIManager()
    {
        for(int i = 0; i < mPanelList.size(); i++)
        {
            delete mPanelList[i];
        }
        mPanelList.clear();
    }

    void EditorGUIManager::Create()
    {
        s_Instance = new D3D12EditorGUIManager();

        s_Instance->InitPanel();
    }


    void EditorGUIManager::Update()
    {
        EngineCore::RenderEngine::GetInstance().OnDrawGUI();
    }
    
    void EditorGUIManager::Render()
    {
        for(int i = 0; i < mPanelList.size(); i++)
        {
            mPanelList[i]->DrawGUI();
        }
		// Rendering
		ImGui::Render();
    }

    void EditorGUIManager::InitPanel()
    {
        EditorPanel* consolePanel = new EditorConsolePanel();
        mPanelList.push_back(consolePanel);
        EditorPanel* viewPanel = new EditorGameViewPanel();
        mPanelList.push_back(viewPanel);
        EditorPanel* hierarchyPanel = new EditorHierarchyPanel();
        mPanelList.push_back(hierarchyPanel);
        EditorPanel* inspectorPanel = new EditorInspectorPanel();
        mPanelList.push_back(inspectorPanel);
        EditorPanel* mainBarPanel = new EditorMainBar();
        mPanelList.push_back(mainBarPanel);
        EditorPanel* proectPanel = new EditorProjectPanel();
        mPanelList.push_back(proectPanel);

    }
}