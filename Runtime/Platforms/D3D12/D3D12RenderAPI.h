#pragma once
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    class D3D12RenderAPI : public RenderAPI
    {
    public:
        virtual void BeginFrame() override;
        virtual void Render() override;
        virtual void EndFrame() override;

        D3D12RenderAPI(){};
        ~D3D12RenderAPI(){};
    };

}