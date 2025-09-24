#include "PreCompiledHeader.h"
#include "FrameBufferManager.h"
#include "RenderAPI.h"

namespace EngineCore
{
    std::unique_ptr<FrameBufferManager> FrameBufferManager::s_Instance = nullptr;
    
    FrameBufferManager::FrameBufferManager()
    {
        mBackBuffer = new FrameBufferObject();
        mBackBuffer->name = "BackBuffer";
    }

    FrameBufferManager::~FrameBufferManager()
    {
        delete mBackBuffer;
    }

    void FrameBufferManager::CreateFBO(const string& name, FrameBufferObject* fboDesc)
    {
        RenderAPI::s_Instance->CreateFBO(name, fboDesc);
        mFBOMap.try_emplace(name, fboDesc);
    }

    FrameBufferObject* FrameBufferManager::GetFBO(const string& name)
    {
        if (mFBOMap.count(name) <= 0) 
        {
            ASSERT_MSG(false, "This FrameBuffer Havent been Created");
            return nullptr;
        }
        return mFBOMap[name];
    }

    void FrameBufferManager::Create()
    {
        FrameBufferManager::s_Instance = std::make_unique<FrameBufferManager>();
    }

} // namespace EngineCore
