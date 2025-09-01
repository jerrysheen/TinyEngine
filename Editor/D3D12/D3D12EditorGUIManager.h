#pragma once
#include "Windows/EditorGUIManager.h"
#include "imgui.h"

namespace EngineEditor
{
    class D3D12EditorGUIManager : public EditorGUIManager
    {
    public:
        virtual void BeginFrame() override;
        virtual void EndFrame() override;
        virtual void Update() override;
        virtual void Render() override;

        D3D12EditorGUIManager();
        ~D3D12EditorGUIManager(){};
    };
}