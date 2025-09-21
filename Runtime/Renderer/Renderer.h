#pragma once
#include "Managers/Manager.h"
#include "RenderPipeLine/RenderPass.h"
#include "Core/PublicStruct.h"
#include "RenderContext.h"

namespace EngineCore
{
    class Renderer : public Manager<Renderer>
    {
    public:
        Renderer();
        ~Renderer(){};
        static void Create();
        void AddRenderPass(RenderPass* renderPass);

        void BeginFrame();
        void Render(const RenderContext& context);
        void EndFrame();
        inline void AddRenderPassInfo(RenderPassInfo* renderpassInfo){mRenderPassInfo.push_back(renderpassInfo);}

    private:
        std::vector<RenderPass*> mRenderPasses;
        std::vector<RenderPassInfo*> mRenderPassInfo;
    };
}