#include "PreCompiledHeader.h"
#include "RenderTexture.h"
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    RenderTexture::RenderTexture(TextureDesc textureDesc)
        :textureDesc(textureDesc)
    {
        textureBuffer = RenderAPI::GetInstance()->CreateRenderTexture(textureDesc);
    }
}