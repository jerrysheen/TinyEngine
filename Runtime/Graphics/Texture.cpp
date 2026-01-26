#include "PreCompiledHeader.h"
#include "Texture.h"
#include "stb_image.h"
#include "Renderer/RenderAPI.h"
#include "Settings/ProjectSettings.h"

namespace EngineCore
{
    Texture::Texture(const string& textureID)
        :textureBuffer(nullptr)
    {
        textureDesc.name = textureID;
    }

    void Texture::OnLoadComplete()
    {
        // 做保护， 避免外部多次访问
        if (textureBuffer && cpuData.size() == 0) return;
        textureBuffer = RenderAPI::GetInstance()->CreateTextureBuffer(static_cast<unsigned char*>(cpuData.data()), textureDesc);
        // 似乎还不能清？为什么
        // 代码逻辑错误。
        cpuData.clear();
        cpuData.shrink_to_fit();
    }

}

