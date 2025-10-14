#pragma once
#include "PreCompiledHeader.h"
#include "Managers/Manager.h"
#include "Core/PublicEnum.h"
#include "Core/PublicStruct.h"

namespace EngineCore
{
    class FrameBufferManager : public Manager<FrameBufferManager>
    {
    public:
        FrameBufferManager();
        ~FrameBufferManager();
        void CreateFBO(FrameBufferObject* fboDesc);
        FrameBufferObject* GetFBO(const string& name);
        static void Create();

        inline FrameBufferObject* GetScreenBuffer(){return mBackBuffer;};
    private:
        unordered_map<string, FrameBufferObject*> mFBOMap;
        FrameBufferObject* mBackBuffer;
    };

}