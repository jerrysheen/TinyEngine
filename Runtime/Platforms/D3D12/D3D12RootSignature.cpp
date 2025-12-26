#include "PreCompiledHeader.h"
#include "D3D12RootSignature.h"
#include "Graphics/Shader.h"
#include "Graphics/ComputeShader.h"

namespace EngineCore
{
    unordered_map<RootSignatureKey, ComPtr<ID3D12RootSignature>, RootSignatureKey> D3D12RootSignature::mRootSigMap;
    
    ComPtr<ID3D12RootSignature> D3D12RootSignature::GetOrCreateARootSig(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, Shader *shader)
    {
        if (mRootSigMap.count(shader->mShaderReflectionInfo.mRootSigKey) > 0) 
        {
            return mRootSigMap[shader->mShaderReflectionInfo.mRootSigKey];
        }

        auto& srvInfo = shader->mShaderReflectionInfo.mTextureInfo;

        vector<CD3DX12_ROOT_PARAMETER> slotRootParameter;
        vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges;
        descriptorRanges.reserve(10);

        slotRootParameter.emplace_back();
        auto indiceBind = GetRootSigBinding(RootSigSlot::DrawIndiceConstant);
        slotRootParameter.back().InitAsConstants(1, indiceBind.RegisterIndex, indiceBind.RegisterSpace);

        slotRootParameter.emplace_back();
        auto frameBind = GetRootSigBinding(RootSigSlot::PerFrameData);
        slotRootParameter.back().InitAsConstantBufferView(frameBind.RegisterIndex, frameBind.RegisterSpace);
        
        slotRootParameter.emplace_back();
        auto passBind = GetRootSigBinding(RootSigSlot::PerPassData);
        slotRootParameter.back().InitAsConstantBufferView(passBind.RegisterIndex, passBind.RegisterSpace);


        slotRootParameter.emplace_back();
        auto allObjectBind = GetRootSigBinding(RootSigSlot::AllObjectData);
        slotRootParameter.back().InitAsShaderResourceView(allObjectBind.RegisterIndex, allObjectBind.RegisterSpace); // register(u0, space0)
        
        slotRootParameter.emplace_back();
        auto allMaterialBind = GetRootSigBinding(RootSigSlot::AllMaterialData);
        slotRootParameter.back().InitAsShaderResourceView(allMaterialBind.RegisterIndex, allMaterialBind.RegisterSpace);
        
        slotRootParameter.emplace_back();
        auto perDrawInstanceList = GetRootSigBinding(RootSigSlot::PerDrawInstanceObjectsList);
        slotRootParameter.back().InitAsShaderResourceView(perDrawInstanceList.RegisterIndex, perDrawInstanceList.RegisterSpace);

        // ⭐ 处理 SRV (纹理) - Root Param 5+
        std::unordered_map<UINT, std::vector<ShaderBindingInfo>> srvBySpace;
        for (auto& bindingInfo : srvInfo)
        {
            srvBySpace[bindingInfo.space].push_back(bindingInfo);
        }

        for (auto& [space, srvList] : srvBySpace)
        {
            std::sort(srvList.begin(), srvList.end(),
                [](const ShaderBindingInfo& a, const ShaderBindingInfo& b) {
                    return a.registerSlot < b.registerSlot;
                });

            UINT rootParamIndex = slotRootParameter.size();

            descriptorRanges.emplace_back();
            descriptorRanges.back().Init(
                D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                srvList.size(),
                srvList[0].registerSlot,  // 使用实际的起始寄存器位置
                space,
                0
            );

            slotRootParameter.emplace_back();
            slotRootParameter.back().InitAsDescriptorTable(1, &descriptorRanges.back());

        }

        // Static Sampler
        CD3DX12_STATIC_SAMPLER_DESC staticSampler(
            0,
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP
        );

        // 创建 Root Signature
        // todo: 根据shader来判断创建 产生一个hash
        ComPtr<ID3D12RootSignature> tempRootSignature;
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
            static_cast<UINT>(slotRootParameter.size()),
            slotRootParameter.data(),
            1, &staticSampler,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        ComPtr<ID3DBlob> serializedRootSig = nullptr;
        ComPtr<ID3DBlob> errorBlob = nullptr;

        ThrowIfFailed(D3D12SerializeRootSignature(
            &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
            &serializedRootSig, &errorBlob));

        ThrowIfFailed(md3dDevice->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&tempRootSignature)));

        mRootSigMap[shader->mShaderReflectionInfo.mRootSigKey] = tempRootSignature;
        return tempRootSignature;
    }

    ComPtr<ID3D12RootSignature> D3D12RootSignature::GetOrCreateAComputeShaderRootSig(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, ComputeShader *csShader)
    {
        const RootSignatureKey& key = csShader->mShaderReflectionInfo.mRootSigKey;
        if (mRootSigMap.count(key) > 0)
        {
            return mRootSigMap[key];
        }

        // Deterministic ordering: CBV -> SRV -> UAV, each sorted by (space, registerSlot).
        auto SortBinding = [](const ShaderBindingInfo& a, const ShaderBindingInfo& b) {
            if (a.space != b.space) return a.space < b.space;
            return a.registerSlot < b.registerSlot;
        };

        std::vector<ShaderBindingInfo> cbvs = csShader->mShaderReflectionInfo.mConstantBufferInfo;
        std::vector<ShaderBindingInfo> srvs = csShader->mShaderReflectionInfo.mTextureInfo;
        std::vector<ShaderBindingInfo> uavs = csShader->mShaderReflectionInfo.mUavInfo;
        std::vector<ShaderBindingInfo> samplersInfo = csShader->mShaderReflectionInfo.mSamplerInfo;
        std::sort(cbvs.begin(), cbvs.end(), SortBinding);
        std::sort(srvs.begin(), srvs.end(), SortBinding);
        std::sort(uavs.begin(), uavs.end(), SortBinding);
        std::sort(samplersInfo.begin(), samplersInfo.end(), SortBinding);

        std::vector<CD3DX12_ROOT_PARAMETER> rootParameters;
        rootParameters.reserve(cbvs.size() + srvs.size() + uavs.size());

        for (const auto& cbv : cbvs)
        {
            rootParameters.emplace_back();
            rootParameters.back().InitAsConstantBufferView(cbv.registerSlot, cbv.space);
        }

        // Note: Root SRV only supports buffers (e.g., StructuredBuffer/ByteAddressBuffer), not textures.
        // This matches current engine's ComputeShader::SetBuffer(name, gpuVA) usage.
        for (const auto& srv : srvs)
        {
            rootParameters.emplace_back();
            rootParameters.back().InitAsShaderResourceView(srv.registerSlot, srv.space);
        }

        for (const auto& uav : uavs)
        {
            rootParameters.emplace_back();
            rootParameters.back().InitAsUnorderedAccessView(uav.registerSlot, uav.space);
        }

        std::vector<CD3DX12_STATIC_SAMPLER_DESC> staticSamplers;
        staticSamplers.reserve(samplersInfo.size());
        for (const auto& samp : samplersInfo)
        {
            CD3DX12_STATIC_SAMPLER_DESC sdesc(
                (UINT)samp.registerSlot,
                D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP
            );
            sdesc.RegisterSpace = (UINT)samp.space;
            sdesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            staticSamplers.push_back(sdesc);
        }

        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
            (UINT)rootParameters.size(),
            rootParameters.empty() ? nullptr : rootParameters.data(),
            (UINT)staticSamplers.size(),
            staticSamplers.empty() ? nullptr : staticSamplers.data(),
            D3D12_ROOT_SIGNATURE_FLAG_NONE
        );

        ComPtr<ID3DBlob> serializedRootSig = nullptr;
        ComPtr<ID3DBlob> errorBlob = nullptr;

        ThrowIfFailed(D3D12SerializeRootSignature(
            &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
            &serializedRootSig, &errorBlob));

        ComPtr<ID3D12RootSignature> rootSig;
        ThrowIfFailed(md3dDevice->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&rootSig)));

        mRootSigMap[key] = rootSig;
        return rootSig;
    }
}