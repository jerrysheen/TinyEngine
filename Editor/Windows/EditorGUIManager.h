#pragma once
#include "PreCompiledHeader.h"

namespace EngineEditor
{
    class EditorGUIManager
    {
    public:
    // 对象不允许纯虚函数，所以要用指针，因为是指针，所以*sInstance表示对象，外面返回引用&就行。
        static EngineEditor::EditorGUIManager* s_Instance;
        static EngineEditor::EditorGUIManager& GetInstance(){ return (*s_Instance);};
        
        static void Create();
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        // Update不需要实现， 因为是一个纯Payload，无任何信息/
        void Update();
        virtual void Render() = 0;

    };


}