#include "PreCompiledHeader.h"
#include "D3D12ShaderUtils.h"
#include "Graphics/Shader.h"
#include "Graphics/ComputeShader.h"
#include "d3dx12.h"  // 确保包含D3D12辅助类
#include "d3dUtil.h"

namespace EngineCore
{
    unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3DBlob>> D3D12ShaderUtils::vsBlobMap;
    unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3DBlob>> D3D12ShaderUtils::psBlobMap;
    unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3DBlob>> D3D12ShaderUtils::csBlobMap;
    bool D3D12ShaderUtils::CompileShaderAndGetReflection(const string &path, Shader *shader)
    {
        shader->name = path;
        Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
        if (!CompileShaderStageAndGetReflection(path, "VSMain", "vs_5_1", shader, ShaderStageType::VERTEX_STAGE, vsBlob))
        {
            cout << "Shader Stage Vertex Compile fail" << endl;
            return false;
        }
        vsBlobMap.try_emplace(shader->GetInstanceID(), vsBlob);
        Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
        if (!CompileShaderStageAndGetReflection(path, "PSMain", "ps_5_1", shader, ShaderStageType::FRAGMENT_STAGE, psBlob))
        {
            cout << "Shader Stage Pixel Shader Compile fail" << endl;
            return false;

        }
        psBlobMap.try_emplace(shader->GetInstanceID(), psBlob);
        return true;
    }

    bool D3D12ShaderUtils::CompileShaderStageAndGetReflection(const string& path, string entryPoint, string target, Shader* shader, ShaderStageType type, Microsoft::WRL::ComPtr<ID3DBlob>& blob)
    {
        ComPtr<ID3DBlob> errorBlob;
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
        std::wstring filename(path.begin(), path.end());

        HRESULT hr = D3DCompileFromFile(
            filename.c_str(),
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint.c_str(),
            target.c_str(),
            compileFlags,
            0,
            &blob,
            &errorBlob
        );

        if (FAILED(hr)) {
            if (errorBlob)
            {
                std::cout << "Compilation Error:\n" << (char*)errorBlob->GetBufferPointer() << std::endl;
            }
            return false;
        }

        ComPtr<ID3D12ShaderReflection>  m_reflection;
        D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_reflection));

        ASSERT(m_reflection != nullptr);
        
        ShaderReflectionInfo* shaderReflectionInfo = &shader->mShaderReflectionInfo;

        D3D12_SHADER_DESC desc;
        m_reflection->GetDesc(&desc);

        // 辅助 Lambda：为了避免写重复的位运算代码
        auto UpdateMask = [&](UINT space, UINT bindPoint, int offset) {
            // 安全检查：防止 bindPoint 太大导致溢出
            // 例如 SRV 给了 32 位，那么 bindPoint 必须 < 32
            // 这里简单做个保护，实际项目可以加 Log
            if (bindPoint >= 32) return;

            uint64_t bit = 1ULL << (offset + bindPoint);

            if (space == 0) {
                shaderReflectionInfo->mRootSigKey.Space0Mask |= bit;
            }
            else if (space == 1) {
                shaderReflectionInfo->mRootSigKey.Space1Mask |= bit;
            }
        };


        for (UINT i = 0; i < desc.BoundResources; ++i) {
            D3D12_SHADER_INPUT_BIND_DESC bindDesc;
            m_reflection->GetResourceBindingDesc(i, &bindDesc);

            // --- 新增：核心位运算逻辑 ---
            // 根据类型直接计算 Mask
            switch (bindDesc.Type) {
            case D3D_SIT_CBUFFER:
                UpdateMask(bindDesc.Space, bindDesc.BindPoint, ShaderReflectionInfo::BIT_OFFSET_CBV);
                break;
            case D3D_SIT_TEXTURE:
                UpdateMask(bindDesc.Space, bindDesc.BindPoint, ShaderReflectionInfo::BIT_OFFSET_SRV);
                break;
            case D3D_SIT_SAMPLER:
                UpdateMask(bindDesc.Space, bindDesc.BindPoint, ShaderReflectionInfo::BIT_OFFSET_SAMPLER);
                break;
            case D3D_SIT_STRUCTURED:
            case D3D_SIT_UAV_RWTYPED:
            case D3D_SIT_UAV_RWSTRUCTURED:
                // ... 其他 UAV 类型 ...
                UpdateMask(bindDesc.Space, bindDesc.BindPoint, ShaderReflectionInfo::BIT_OFFSET_UAV);
                break;
            }

            switch (bindDesc.Type) {
            case D3D_SIT_CBUFFER:
            {
                for (auto& x : shaderReflectionInfo->mConstantBufferInfo)
                {
                    if (x.resourceName == bindDesc.Name) break;
                }
                shaderReflectionInfo->mConstantBufferInfo.emplace_back(bindDesc.Name, ShaderResourceType::CONSTANT_BUFFER, bindDesc.BindPoint, 0, bindDesc.Space, bindDesc.BindCount);

                break;
            }
            case D3D_SIT_TEXTURE:
                for (auto& x : shaderReflectionInfo->mTextureInfo)
                {
                    if (x.resourceName == bindDesc.Name) break;
                }
                shaderReflectionInfo->mTextureInfo.emplace_back(bindDesc.Name, ShaderResourceType::TEXTURE, bindDesc.BindPoint, 0, bindDesc.Space, bindDesc.BindCount);
                break;
            case D3D_SIT_SAMPLER:
                for (auto& x : shaderReflectionInfo->mSamplerInfo)
                {
                    if (x.resourceName == bindDesc.Name) break;
                }
                shaderReflectionInfo->mSamplerInfo.emplace_back(bindDesc.Name, ShaderResourceType::SAMPLER, bindDesc.BindPoint, 0, bindDesc.Space, bindDesc.BindCount);
                break;
            case D3D_SIT_STRUCTURED:
            case D3D_SIT_UAV_RWTYPED:
            case D3D_SIT_UAV_RWSTRUCTURED:
            case D3D_SIT_UAV_RWBYTEADDRESS:
            case D3D_SIT_UAV_APPEND_STRUCTURED:
            case D3D_SIT_UAV_CONSUME_STRUCTURED:
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                for (auto& x : shaderReflectionInfo->mUavInfo)
                {
                    if (x.resourceName == bindDesc.Name) break;
                }
                shaderReflectionInfo->mUavInfo.emplace_back(bindDesc.Name, ShaderResourceType::UAV, bindDesc.BindPoint, 0, bindDesc.Space, bindDesc.BindCount);
                break;
            default:
                std::cout << " Not find any exites shader resource type " << std::endl;
                break;
            }
        }
        
        if (type == ShaderStageType::VERTEX_STAGE) 
        {
            std::cout << "\n=== Input Layout ===\n";
            for (UINT i = 0; i < desc.InputParameters; ++i) {
                D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
                m_reflection->GetInputParameterDesc(i, &paramDesc);
                if (strcmp(paramDesc.SemanticName, "POSITION") == 0) 
                {
                    shader->mShaderInputLayout.emplace_back(VertexAttribute::POSITION);
                }
                else if (strcmp(paramDesc.SemanticName, "NORMAL") == 0) 
                {
                    shader->mShaderInputLayout.emplace_back(VertexAttribute::NORMAL);
                }
                else if (strcmp(paramDesc.SemanticName, "TEXCOORD") == 0) 
                {
                    // 目前先比较 TexCoord一个， 没有涉及index
                    shader->mShaderInputLayout.emplace_back(VertexAttribute::UV0);
                }
            }
        }
        return true;
    }

    
    bool D3D12ShaderUtils::CompileComputeShaderAndGetReflection(const string &path, ComputeShader *csShader)
    {
         // 编译
         ComPtr<ID3DBlob> computeShaderBlob;
         ComPtr<ID3DBlob> errorBlob;
         std::wstring filename(path.begin(), path.end());
         HRESULT hr = D3DCompileFromFile(
             filename.c_str(),
             nullptr,                // Defines
             D3D_COMPILE_STANDARD_FILE_INCLUDE, // Include handler
             "CSMain",                 // 入口函数
             "cs_5_1",               // Target Profile
             D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // Flags (Debug用)
             0,
             &computeShaderBlob,
             &errorBlob
         );
         if (FAILED(hr)) {
            if (errorBlob)
            {
                std::cout << "Compilation Error:\n" << (char*)errorBlob->GetBufferPointer() << std::endl;
            }
            return false;
         }
         
         csBlobMap.try_emplace(csShader->GetInstanceID(), computeShaderBlob);
         ComPtr<ID3D12ShaderReflection> pReflector;
     
         // 1. 从编译好的 Shader Blob 创建反射接口
         hr = D3DReflect(
             computeShaderBlob->GetBufferPointer(),
             computeShaderBlob->GetBufferSize(),
             IID_PPV_ARGS(&pReflector)
         );
     
         if (FAILED(hr)) return false;
     
         // 2. 获取 Shader 的整体描述
         D3D12_SHADER_DESC shaderDesc;
         pReflector->GetDesc(&shaderDesc);
         
         // shaderDesc.BoundResources 是资源的数量
         printf("Resource Count: %d\n", shaderDesc.BoundResources);
         ShaderReflectionInfo* shaderReflectionInfo = &csShader->mShaderReflectionInfo;
 
         // 辅助 Lambda：生成 RootSigKey（与 graphics 反射保持一致）
         auto UpdateMask = [&](UINT space, UINT bindPoint, int offset) {
             // 保护：避免 shift 溢出/UB
             if ((uint64_t)offset + (uint64_t)bindPoint >= 64ULL) return;
             uint64_t bit = 1ULL << ((uint64_t)offset + (uint64_t)bindPoint);
 
             if (space == 0) {
                 shaderReflectionInfo->mRootSigKey.Space0Mask |= bit;
             }
             else if (space == 1) {
                 shaderReflectionInfo->mRootSigKey.Space1Mask |= bit;
             }
         };
 
         // 3. 遍历所有绑定的资源 (CBV, SRV, UAV)
         for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
         {
             D3D12_SHADER_INPUT_BIND_DESC bindDesc;
             pReflector->GetResourceBindingDesc(i, &bindDesc);
     
             printf("Name: %s\n", bindDesc.Name); // HLSL 里的变量名，如 "g_Output"
             printf("Type: %d\n", bindDesc.Type); // 如 D3D_SIT_UAV_RWTYPED
             printf("BindPoint: %d\n", bindDesc.BindPoint); // 对应的寄存器号，如 u0 中的 0
             printf("Space: %d\n", bindDesc.Space); // register(u0, space1) 中的 1
 
             // RootSigKey bitmask
             switch (bindDesc.Type)
             {
                 case D3D_SIT_CBUFFER:
                     UpdateMask(bindDesc.Space, bindDesc.BindPoint, ShaderReflectionInfo::BIT_OFFSET_CBV);
                     break;
                 case D3D_SIT_TEXTURE:
                 case D3D_SIT_STRUCTURED:
                 case D3D_SIT_BYTEADDRESS:
                 case D3D_SIT_TBUFFER:
                     UpdateMask(bindDesc.Space, bindDesc.BindPoint, ShaderReflectionInfo::BIT_OFFSET_SRV);
                     break;
                 case D3D_SIT_SAMPLER:
                     UpdateMask(bindDesc.Space, bindDesc.BindPoint, ShaderReflectionInfo::BIT_OFFSET_SAMPLER);
                     break;
                 case D3D_SIT_UAV_RWTYPED:
                 case D3D_SIT_UAV_RWSTRUCTURED:
                 case D3D_SIT_UAV_RWBYTEADDRESS:
                 case D3D_SIT_UAV_APPEND_STRUCTURED:
                 case D3D_SIT_UAV_CONSUME_STRUCTURED:
                 case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                     UpdateMask(bindDesc.Space, bindDesc.BindPoint, ShaderReflectionInfo::BIT_OFFSET_UAV);
                     break;
                 default:
                     break;
             }
 
             switch (bindDesc.Type) {
                 case D3D_SIT_CBUFFER:
                 {
                     for (auto& x : shaderReflectionInfo->mConstantBufferInfo)
                     {
                         if (x.resourceName == bindDesc.Name) break;
                     }
                     shaderReflectionInfo->mConstantBufferInfo.emplace_back(bindDesc.Name, ShaderResourceType::CONSTANT_BUFFER, bindDesc.BindPoint, 0, bindDesc.Space);
     
                     break;
                 }
                 case D3D_SIT_TEXTURE:
                 case D3D_SIT_STRUCTURED:
                 case D3D_SIT_BYTEADDRESS:
                 case D3D_SIT_TBUFFER:
                     for (auto& x : shaderReflectionInfo->mTextureInfo)
                     {
                         if (x.resourceName == bindDesc.Name) break;
                     }
                     shaderReflectionInfo->mTextureInfo.emplace_back(bindDesc.Name, ShaderResourceType::TEXTURE, bindDesc.BindPoint, 0, bindDesc.Space);
                     break;
                 case D3D_SIT_SAMPLER:
                     for (auto& x : shaderReflectionInfo->mSamplerInfo)
                     {
                         if (x.resourceName == bindDesc.Name) break;
                     }
                     shaderReflectionInfo->mSamplerInfo.emplace_back(bindDesc.Name, ShaderResourceType::SAMPLER, bindDesc.BindPoint, 0, bindDesc.Space);
                     break;
                 case D3D_SIT_UAV_RWTYPED:
                 case D3D_SIT_UAV_RWSTRUCTURED:
                 case D3D_SIT_UAV_RWBYTEADDRESS:
                 case D3D_SIT_UAV_APPEND_STRUCTURED:
                 case D3D_SIT_UAV_CONSUME_STRUCTURED:
                 case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                     for (auto& x : shaderReflectionInfo->mUavInfo)
                     {
                         if (x.resourceName == bindDesc.Name) break;
                     }
                     shaderReflectionInfo->mUavInfo.emplace_back(bindDesc.Name, ShaderResourceType::UAV, bindDesc.BindPoint, 0, bindDesc.Space);
                     break;
                 default:
                     std::cout << " Not find any exites shader resource type " << std::endl;
                     break;
                 }
         }
    }
}