#include "PreCompiledHeader.h"
#include "D3D12RootSignature.h"
#include "Graphics/Shader.h"
#include "Graphics/ComputeShader.h"
#include "Settings/ProjectSettings.h"

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

        // 执行空绑定
        if(srvBySpace.size() == 0)
        {
            descriptorRanges.emplace_back();
            descriptorRanges.back().Init(
                D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1,
                0,  // 使用实际的起始寄存器位置
                0,
                0
            );

            slotRootParameter.emplace_back();
            slotRootParameter.back().InitAsDescriptorTable(1, &descriptorRanges.back());

        }
        for (auto& [space, srvList] : srvBySpace)
        {
            std::sort(srvList.begin(), srvList.end(),
                [](const ShaderBindingInfo& a, const ShaderBindingInfo& b) {
                    return a.registerSlot < b.registerSlot;
                });

            UINT rootParamIndex = slotRootParameter.size();

            UINT numDescriptors = 0;
            for(const auto& info : srvList)
            {
                if(info.bindCount == 0) 
                {
                    numDescriptors = UINT_MAX;
                    break;
                }
                numDescriptors += info.bindCount;
            }

            descriptorRanges.emplace_back();
            descriptorRanges.back().Init(
                D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                numDescriptors,
                srvList[0].registerSlot,  // 使用实际的起始寄存器位置
                space,
                0
            );

            slotRootParameter.emplace_back();
            slotRootParameter.back().InitAsDescriptorTable(1, &descriptorRanges.back());

        }

        if(RenderSettings::s_EnableVertexPulling)
        {
            slotRootParameter.emplace_back();
            auto largeVertexBuffer = GetRootSigBinding(RootSigSlot::LargeVertexBuffer);
            slotRootParameter.back().InitAsShaderResourceView(largeVertexBuffer.RegisterIndex, largeVertexBuffer.RegisterSpace);
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

        std::vector<ShaderBindingInfo> buffers = csShader->mShaderReflectionInfo.mBufferInfo;
        std::vector<ShaderBindingInfo> textures = csShader->mShaderReflectionInfo.mTextureInfo;
        std::vector<ShaderBindingInfo> samplers = csShader->mShaderReflectionInfo.mSamplerInfo;
        std::sort(buffers.begin(), buffers.end(), SortBinding);
        std::sort(textures.begin(), textures.end(), SortBinding);
        std::sort(samplers.begin(), samplers.end(), SortBinding);

        std::vector<CD3DX12_ROOT_PARAMETER> rootParameters;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges;
        rootParameters.reserve(buffers.size() + 2);
        descriptorRanges.reserve(2);

        for (const auto& buffer : buffers)
        {
            if(buffer.type == ShaderResourceType::CONSTANT_BUFFER)
            {
                rootParameters.emplace_back();
                rootParameters.back().InitAsConstantBufferView(buffer.registerSlot, buffer.space);
            }
            else if(buffer.type == ShaderResourceType::SRV_BUFFER)
            {
                rootParameters.emplace_back();
                rootParameters.back().InitAsShaderResourceView(buffer.registerSlot, buffer.space);
            }
            else if(buffer.type == ShaderResourceType::UAV_BUFFER)
            {
                rootParameters.emplace_back();
                rootParameters.back().InitAsUnorderedAccessView(buffer.registerSlot, buffer.space);
            }
        }

        // --------------------------
        // 新增：3个固定描述符表参数（所有ComputeShader都有，不管有没有纹理）
        // --------------------------
        const int MAX_TEXTURE_SRV = 16;   // 最多支持16个纹理SRV
        const int MAX_TEXTURE_UAV = 16;   // 最多支持16个纹理UAV

        // 4. 纹理SRV描述符表（Space1，t0~t15）
        {
            descriptorRanges.emplace_back();
            descriptorRanges.back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_TEXTURE_SRV, 0, 1); // BaseReg:0, Space:1
            rootParameters.emplace_back();
            rootParameters.back().InitAsDescriptorTable(1, &descriptorRanges.back(), D3D12_SHADER_VISIBILITY_ALL);
        }

        // 5. 纹理UAV描述符表（Space1，u0~u15）
        {
            descriptorRanges.emplace_back();
            descriptorRanges.back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, MAX_TEXTURE_UAV, 0, 1); // BaseReg:0, Space:1
            rootParameters.emplace_back();
            rootParameters.back().InitAsDescriptorTable(1, &descriptorRanges.back(), D3D12_SHADER_VISIBILITY_ALL);
        }

        std::vector<CD3DX12_STATIC_SAMPLER_DESC> staticSamplers;
        staticSamplers.reserve(samplers.size());
        for (const auto& samp : samplers)
        {

            CD3DX12_STATIC_SAMPLER_DESC sdesc = GetSamplerDescByName(samp.resourceName, samp.registerSlot);
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

    ComPtr<ID3D12RootSignature> D3D12RootSignature::GetOrCreateGPUSceneGraphicsRootSig(ComPtr<ID3D12Device> device)
    {
        std::vector<CD3DX12_ROOT_PARAMETER> rootParams;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
        rootParams.reserve(8);
        ranges.reserve(1);
  
        // 0. DrawIndiceConstant : b0, space0
        rootParams.emplace_back();
        auto indiceBind = GetRootSigBinding(RootSigSlot::DrawIndiceConstant);
        rootParams.back().InitAsConstants(1, indiceBind.RegisterIndex, indiceBind.RegisterSpace);
  
        // 1. PerFrameData : b1, space0
        rootParams.emplace_back();
        auto frameBind = GetRootSigBinding(RootSigSlot::PerFrameData);
        rootParams.back().InitAsConstantBufferView(frameBind.RegisterIndex, frameBind.RegisterSpace);
  
        // 2. PerPassData : b2, space0
        rootParams.emplace_back();
        auto passBind = GetRootSigBinding(RootSigSlot::PerPassData);
        rootParams.back().InitAsConstantBufferView(passBind.RegisterIndex, passBind.RegisterSpace);
  
        // 3. AllObjectData : t0, space1
        rootParams.emplace_back();
        auto allObjectBind = GetRootSigBinding(RootSigSlot::AllObjectData);
        rootParams.back().InitAsShaderResourceView(allObjectBind.RegisterIndex, allObjectBind.RegisterSpace);
  
        // 4. AllMaterialData : t1, space1
        rootParams.emplace_back();
        auto allMaterialBind = GetRootSigBinding(RootSigSlot::AllMaterialData);
        rootParams.back().InitAsShaderResourceView(allMaterialBind.RegisterIndex, allMaterialBind.RegisterSpace);
  
        // 5. PerDrawInstanceObjectsList : t2, space1
        rootParams.emplace_back();
        auto perDrawBind = GetRootSigBinding(RootSigSlot::PerDrawInstanceObjectsList);
        rootParams.back().InitAsShaderResourceView(perDrawBind.RegisterIndex, perDrawBind.RegisterSpace);
  
        // 6. Textures : t0, space0 descriptor table
        ranges.emplace_back();
        ranges.back().Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1024,   // 对齐 g_Textures[1024]
            0,
            0,
            0);
  
        rootParams.emplace_back();
        rootParams.back().InitAsDescriptorTable(1, &ranges.back());
  
        // 7. LargeVertexBuffer : t3, space1
        if (RenderSettings::s_EnableVertexPulling)
        {
            rootParams.emplace_back();
            auto largeVB = GetRootSigBinding(RootSigSlot::LargeVertexBuffer);
            rootParams.back().InitAsShaderResourceView(largeVB.RegisterIndex, largeVB.RegisterSpace);
        }
  
        CD3DX12_STATIC_SAMPLER_DESC staticSampler(
            0,
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);
  
        CD3DX12_ROOT_SIGNATURE_DESC desc(
            (UINT)rootParams.size(),
            rootParams.data(),
            1,
            &staticSampler,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
  
        ComPtr<ID3DBlob> serialized;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(
            &desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error));
  
        ComPtr<ID3D12RootSignature> rootSig;
        ThrowIfFailed(device->CreateRootSignature(
            0,
            serialized->GetBufferPointer(),
            serialized->GetBufferSize(),
            IID_PPV_ARGS(&rootSig)));
  
        return rootSig;
    }


    CD3DX12_STATIC_SAMPLER_DESC D3D12RootSignature::GetSamplerDescByName(const std::string& name, uint32_t registerSlot)
    {
        if (name == "sampler_point_clamp")
        {
            CD3DX12_STATIC_SAMPLER_DESC desc(
                registerSlot,
                D3D12_FILTER_MIN_MAG_MIP_POINT,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP
                );
            return desc;
        }
        else if (name == "sampler_linear_wrap")
        {
            CD3DX12_STATIC_SAMPLER_DESC desc(
                registerSlot,
                D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP
            );
            return desc;
        }
        else if (name == "sampler_linear_clamp")
        {
            CD3DX12_STATIC_SAMPLER_DESC desc(
                registerSlot,
                D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP
            );
            return desc;
        }
        else if (name == "sampler_anisotropic_wrap")
        {
            CD3DX12_STATIC_SAMPLER_DESC desc(
                registerSlot,
                D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP
            );
            return desc;
        }
        ASSERT("Name Not Match");
        return CD3DX12_STATIC_SAMPLER_DESC();
    }
}
