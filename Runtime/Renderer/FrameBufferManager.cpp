#include "PreCompiledHeader.h"
#include "FrameBufferManager.h"
#include "RenderAPI.h"

namespace EngineCore
{
    std::unique_ptr<FrameBufferManager> FrameBufferManager::s_Instance = nullptr;
    
    FrameBufferManager::FrameBufferManager()
    {
        FrameBufferManager::s_Instance = std::make_unique<FrameBufferManager>();
    }

    FrameBufferObject* FrameBufferManager::CreateFBO(const string& name, const FrameBufferObject& fboDesc)
    {
        auto fbo = RenderAPI::s_Instance->CreateFBO(name, fboDesc);
        mFBOMap.try_emplace(name, fbo);
        return fbo;
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

} // namespace EngineCore
