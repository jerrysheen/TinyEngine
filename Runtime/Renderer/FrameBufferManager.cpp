#include "PreCompiledHeader.h"
#include "FrameBufferManager.h"
#include "RenderAPI.h"
#include "GameObject/Component.h"

namespace EngineCore
{
    std::unique_ptr<FrameBufferManager> FrameBufferManager::s_Instance = nullptr;
    
    FrameBufferManager::FrameBufferManager()
    {
        mBackBuffer = ResourceManager::GetInstance()->CreateResource<FrameBufferObject>("BackBuffer");
    }

    FrameBufferManager::~FrameBufferManager()
    {
    }

    ResourceHandle<FrameBufferObject> FrameBufferManager::CreateFBO(const FrameBufferDesc& fboDesc)
    {
        ResourceHandle<FrameBufferObject> fbo = 
            ResourceManager::GetInstance()->CreateResource<FrameBufferObject>(fboDesc);
        RenderAPI::s_Instance->CreateFBO(fbo.Get());
        mFBOMap.try_emplace(fboDesc.name, fbo);
        return fbo;
    }

    ResourceHandle<FrameBufferObject> FrameBufferManager::GetFBO(const string& name)
    {
        if (mFBOMap.count(name) <= 0) 
        {
            ASSERT_MSG(false, "This FrameBuffer Havent been Created");
            return ResourceHandle<FrameBufferObject>(0);
        }
        return mFBOMap[name];
    }

    void FrameBufferManager::Create()
    {
        FrameBufferManager::s_Instance = std::make_unique<FrameBufferManager>();
    }

} // namespace EngineCore
