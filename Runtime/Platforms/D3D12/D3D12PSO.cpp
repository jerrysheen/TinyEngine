#include "PreCompiledHeader.h"
#include "D3D12PSO.h"
#include "D3D12Struct.h"
#include "Renderer/RenderCommand.h"
#include "D3D12ShaderUtils.h"
#include "D3D12RootSignature.h"

namespace EngineCore
{
    unordered_map<uint32_t, ComPtr<ID3D12PipelineState>> D3D12PSO::shaderPSOMap;
    ComPtr<ID3D12PipelineState> D3D12PSO::CreatePSO(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, const TD3D12PSO& pso)
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

    ComPtr<ID3D12PipelineState> D3D12PSO::GetOrCreatePSO(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, PSODesc &psodesc)
    {
        uint32_t psoHash = psodesc.GetHash();
        if(shaderPSOMap.count(psoHash) > 0)
        {
            return shaderPSOMap[psoHash];
        }
        TD3D12PSO pso;
        pso.desc = psodesc;
        pso.inputLayout =         
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        pso.psBlob = D3D12ShaderUtils::GetPSBlob(psodesc.matRenderState.shaderInstanceID);
        pso.vsBlob = D3D12ShaderUtils::GetVSBlob(psodesc.matRenderState.shaderInstanceID);
        pso.rootSignature = D3D12RootSignature::GetOrCreateARootSig(psodesc.matRenderState.rootSignatureKey);
        ComPtr<ID3D12PipelineState> temp = CreatePSO(md3dDevice, pso);
        shaderPSOMap.try_emplace(psoHash, temp);
        return temp;
    }

    ComPtr<ID3D12PipelineState> D3D12PSO::GetOrCreateComputeShaderPSO(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, PSODesc& psodesc)
    {
        uint32_t psoHash = psodesc.GetHash();
        if(shaderPSOMap.count(psoHash) > 0)
        {
            return shaderPSOMap[psoHash];
        }

        ComPtr<ID3D12RootSignature> rootSig =
        D3D12RootSignature::GetOrCreateARootSig(psodesc.matRenderState.rootSignatureKey);
        ComPtr<ID3DBlob> csBlob =
            D3D12ShaderUtils::GetCSBlob(psodesc.matRenderState.shaderInstanceID);

        // 3) 组 Compute PSO 描述
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = rootSig.Get();
        desc.CS = { csBlob->GetBufferPointer(), csBlob->GetBufferSize() };
        desc.NodeMask = 0;
        desc.CachedPSO = { nullptr, 0 };
        desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        ComPtr<ID3D12PipelineState> pso;
        ThrowIfFailed(md3dDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pso)));
    
        shaderPSOMap.emplace(psoHash, pso);
        return pso;
    }

    D3D12_DEPTH_STENCIL_DESC D3D12PSO::GetDepthStencilDesc(const MaterailRenderState &matRenderState)
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

    D3D12_BLEND_DESC D3D12PSO::GetBlendDesc(const MaterailRenderState &matRenderState)
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

    D3D12_RASTERIZER_DESC D3D12PSO::GetRasterizerDesc(const MaterailRenderState &matRenderState)
    {
        D3D12_RASTERIZER_DESC rastDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        // 根据需要设置裁剪模式等
        return rastDesc;
    }
}
