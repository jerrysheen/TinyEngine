#pragma once
#include "RenderPass.h"
#include "Core/PublicStruct.h"
#include "Renderer/RenderContext.h"

namespace EngineCore
{
    class OpaqueRenderPass : public RenderPass
    {
    public:

        OpaqueRenderPass();
        // renderpass 初始化
        //virtual void Create(const RenderContext& context) override;
        // todo Create 也需要参数
        virtual void Create() override;
        // 配置rt等
        virtual void Configure(const RenderContext& context) override;
        
        virtual void Prepare(RenderContext& context) override;


        // 执行具体的draw
        virtual void Execute(RenderContext& context) override;

        virtual void Filter(const RenderContext& context) override;

        virtual void Submit() override;
    };

}
