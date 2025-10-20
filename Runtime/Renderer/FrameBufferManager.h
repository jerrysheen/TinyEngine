#pragma once
#include "PreCompiledHeader.h"
#include "Managers/Manager.h"
#include "Core/PublicEnum.h"
#include "Core/PublicStruct.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/FrameBufferObject.h"

namespace EngineCore
{
    class FrameBufferManager : public Manager<FrameBufferManager>
    {
    public:
        FrameBufferManager();
        ~FrameBufferManager();
        ResourceHandle<FrameBufferObject> CreateFBO(const FrameBufferDesc& fboDesc);
        ResourceHandle<FrameBufferObject> GetFBO(const string& name);
        static void Create();

        inline ResourceHandle<FrameBufferObject> GetScreenBuffer(){return mBackBuffer;};
    private:
        unordered_map<string, ResourceHandle<FrameBufferObject>> mFBOMap;
        ResourceHandle<FrameBufferObject> mBackBuffer;
    };

}