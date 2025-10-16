#include "PreCompiledHeader.h"
#include "D3D12PSOManager.h"
#include "D3D12Struct.h"

namespace EngineCore
{
    D3D12PSOManager* D3D12PSOManager::sInstance = nullptr;

    D3D12PSOManager::D3D12PSOManager(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice)
    :md3dDevice(d3dDevice)
    {
    }

    void D3D12PSOManager::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice)
    {
        sInstance = new D3D12PSOManager(d3dDevice);
    }

    void D3D12PSOManager::Delete()
    {
        delete sInstance;
    }

    ComPtr<ID3D12PipelineState> D3D12PSOManager::CreatePSO(const TD3D12PSO &pso)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

        psoDesc.pRootSignature = pso.rootSignature.Get();
        psoDesc.VS = { pso.vsBlob->GetBufferPointer(), pso.vsBlob->GetBufferSize() };
        psoDesc.PS = { pso.psBlob->GetBufferPointer(), pso.psBlob->GetBufferSize() };
        psoDesc.InputLayout = { pso.inputLayout.data(), (UINT)pso.inputLayout.size() };
        
        // 根据PSODesc设置状态
        psoDesc.DepthStencilState = GetDepthStencilDesc(pso.desc.matRenderState);
        psoDesc.BlendState = GetBlendDesc(pso.desc.matRenderState);
        psoDesc.RasterizerState = GetRasterizerDesc(pso.desc.matRenderState);

        // 固定配置
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        psoDesc.RTVFormats[0] = pso.desc.colorAttachment == TextureFormat::EMPTY 
                            ? DXGI_FORMAT_UNKNOWN 
                            : DXGI_FORMAT_R8G8B8A8_UNORM; 
        psoDesc.DSVFormat = pso.desc.depthAttachment == TextureFormat::EMPTY
                            ? DXGI_FORMAT_UNKNOWN
                            : DXGI_FORMAT_D24_UNORM_S8_UINT;

        // 创建PSO
        ComPtr<ID3D12PipelineState> d3d12Pso;
        ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&d3d12Pso)));
        
        return d3d12Pso;
    }

    D3D12_DEPTH_STENCIL_DESC D3D12PSOManager::GetDepthStencilDesc(const MaterailRenderState &matRenderState)
    {
        D3D12_DEPTH_STENCIL_DESC desc = {};
        
        desc.DepthEnable = matRenderState.enableDepthTest;
        desc.DepthWriteMask = matRenderState.enableDepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        switch(matRenderState.depthComparisonFunc)
        {
            case DepthComparisonFunc::LESS: desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; break;
            case DepthComparisonFunc::LEQUAL: desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; break;
            case DepthComparisonFunc::EQUAL: desc.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL; break;
            case DepthComparisonFunc::GREATEQUAL: desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL; break;
            case DepthComparisonFunc::GREAT: desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER; break;
            default: desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; break;
        }

        desc.StencilEnable = FALSE;
        return desc;
    }

    D3D12_BLEND_DESC D3D12PSOManager::GetBlendDesc(const MaterailRenderState &matRenderState)
    {
        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;
        
        auto& rt = blendDesc.RenderTarget[0];
        rt.BlendEnable = matRenderState.enableBlend;
        rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        
        if (matRenderState.enableBlend)
        {
            // todo 更加复杂的blend模式。
            rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
            rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            rt.BlendOp = D3D12_BLEND_OP_ADD;
            rt.SrcBlendAlpha = D3D12_BLEND_ONE;
            rt.DestBlendAlpha = D3D12_BLEND_ZERO;
            rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        }
        return blendDesc;
    }

    D3D12_RASTERIZER_DESC D3D12PSOManager::GetRasterizerDesc(const MaterailRenderState &matRenderState)
    {
        D3D12_RASTERIZER_DESC rastDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        // 根据需要设置裁剪模式等
        return rastDesc;
    }
}
