#include "PreCompiledHeader.h"
#include "EditorGUIManager.h"
#include "D3D12/D3D12EditorGUIManager.h"


namespace EngineEditor
{
    EditorGUIManager* EditorGUIManager::s_Instance = nullptr;
    
    void EditorGUIManager::Create()
    {
        s_Instance = new D3D12EditorGUIManager();
    }
}