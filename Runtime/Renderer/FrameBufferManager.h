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
        ~FrameBufferManager(){};
        FrameBufferObject* CreateFBO(const string& name, const FrameBufferObject& fboDesc );
        FrameBufferObject* GetFBO(const string& name);

    private:
        unordered_map<string, FrameBufferObject*> mFBOMap;
    };

}