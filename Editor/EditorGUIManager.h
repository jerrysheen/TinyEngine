#pragma once
#include "GameObject/GameObject.h"

namespace EngineEditor
{
    using GameObject = EngineCore::GameObject;
    class EditorPanel;
    class EditorGUIManager
    {
    public:
    // 对象不允许纯虚函数，所以要用指针，因为是指针，所以*sInstance表示对象，外面返回引用&就行。
        static EngineEditor::EditorGUIManager* s_Instance;
        static EngineEditor::EditorGUIManager* GetInstance(){ return (s_Instance);};
        virtual ~EditorGUIManager();
        
        static void Create();
        static void OnDestory();
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        // Update不需要实现， 因为是一个纯Payload，无任何信息/
        void Update();
        // 录入指令部分，不需要有具体RenderAPI实现。
        void Render();
        void InitPanel();

        inline void SetCurrentSelected(GameObject* go) 
        { 
            currentSelected = go; 
        };
        inline GameObject* GetCurrentSelected()
        { 
            return currentSelected;
        };
    private:
        std::vector<EditorPanel*> mPanelList;
        GameObject* currentSelected = nullptr;
    };


}