#pragma once
#include "d3dUtil.h"
#include "Graphics/Shader.h"
#include "Renderer/RenderStruct.h"
#include "CoreAssert.h"
namespace EngineCore
{
//   Pram Index	    类型	          HLSL 对应	            用途	                    更新频率
//      0	    Root Constants	    b0, space0	        Draw Constants          	极高 (Per Draw)
//      1	    CBV (Root)	        b1, space0	        Per Pass Data           	高 (Per Pass)
//      2	    CBV (Root)	        b2, space0	        Per Frame Data          	中 (Per Frame)
//      3	    Descriptor Table	t0...tN, space0	    Material Textures           高 (Per Material)
//      4	    Descriptor Table	s0...sN, space0	    Samplers                    低
//      5	    SRV (Root/Table)	t0, space1	        Global Object Data      	极低 (Per Scene)
//      6	    SRV (Root/Table)	t1, space1	        Global Material Data    	极低 (Per Scene)
    class D3D12RootSignature
    {
    public:
    // 第三行为比较函数
        static unordered_map<RootSignatureKey, ComPtr<ID3D12RootSignature>, RootSignatureKey> mRootSigMap;
        
        static ComPtr<ID3D12RootSignature> GetOrCreateARootSig(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, Shader* shader)
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
    
        static ComPtr<ID3D12RootSignature> GetOrCreateARootSig(const RootSignatureKey& key)
        {
            ASSERT(mRootSigMap.count(key) > 0);
            return mRootSigMap[key];
        }
    };
}