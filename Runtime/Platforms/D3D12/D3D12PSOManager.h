#pragma once
#include "PreCompiledHeader.h"
#include "d3dUtil.h"
#include "Renderer/RenderCommand.h"
#include "D3D12Struct.h"

namespace EngineCore
{
    class D3D12PSOManager
    {
    public:
        D3D12PSOManager(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice);
        static void Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice);
        static void Delete();
        static D3D12PSOManager* GetInstance()
        {
            ASSERT_MSG(sInstance != nullptr, "D3D12PSOManager not Created!");
            return sInstance;
        };

        ComPtr<ID3D12PipelineState> CreatePSO(const TD3D12PSO &pso);
    private:
        static D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc(const MaterailRenderState& matRenderState);
        static D3D12_BLEND_DESC GetBlendDesc(const MaterailRenderState& matRenderState);
        static D3D12_RASTERIZER_DESC GetRasterizerDesc(const MaterailRenderState& matRenderState);

        Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
        static D3D12PSOManager* sInstance;
    };

}