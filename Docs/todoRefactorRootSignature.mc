完美！您的理解完全正确！让我整理一个完整的重构清单，确保每个步骤都清晰：

## 📋 重构清单（按顺序执行）

### ✅ 步骤 1：Shader 中使用 space 标记更新频率

**当前状态**：
```hlsl
// Assets/Shader/SimpleTestShader.hlsl
cbuffer PerFrameData : register(b0, space0) { ... }     // ✅ 已有
cbuffer PerMaterialData : register(b1, space1) { ... }  // ✅ 已有
cbuffer PerDrawData : register(b2, space2) { ... }      // ✅ 已有

Texture2D DiffuseTexture : register(t0);  // ❌ 需要补充 space
```

**需要修改**：为纹理补充 space 信息
```hlsl
Texture2D DiffuseTexture : register(t0, space1);     // PerMaterial
Texture2D NormalTexture : register(t1, space1);      // PerMaterial
Texture2D SpecularTexture : register(t2, space1);    // PerMaterial
TextureCube EnvironmentMap : register(t3, space0);   // PerFrame
```

---

### ✅ 步骤 2：Shader Reflection 中记录 space

**需要修改的文件**：`Runtime/Core/PublicStruct.h`

让我先看一下当前的结构定义：

[1 tool called]

太好了！我看到 `ShaderBindingInfo ` 已经有 `space` 字段了（第53行）！✅

**但有个小问题需要修正**（第54行构造函数语法错误）：

```cpp
// 当前（第54行）：
struct ShaderBindingInfo (const string& resourceName, ...)  // ❌ 多了 struct

// 应该改为：
ShaderBindingInfo (const string& resourceName, ShaderResourceType type, 
                   int registerSlot, int size = 0, int space = 0)  // ✅
    : resourceName(resourceName), type(type), registerSlot(registerSlot), 
      size(size), space(space)
{}
```

**需要在 `ShaderReflectionInfo` 中添加布局信息**（用于 SetMaterial）：

```cpp
// 在 ShaderReflectionInfo 结构中添加（第60行附近）
struct TableLayoutInfo
{
    D3D12_DESCRIPTOR_RANGE_TYPE type;  // CBV/SRV/SAMPLER
    UINT space;                         // space0/1/2
    UINT rootParamIndex;                // 对应的 root parameter index
};

struct ShaderReflectionInfo
{
    // ... 现有字段
    vector<TableLayoutInfo> mRootParamLayout;  // 【新增】记录 root signature 布局
};
```

**在 reflection 时提取 space**（修改 `D3D12RenderAPI.cpp` 第589-629行）：

[1 tool called]

看到了！需要在第608、616、623行添加 `bindDesc.Space`：

```cpp
// 第608行 - CBV
ShaderReflectionInfo->mBufferInfo.emplace_back(
    bindDesc.Name, ShaderResourceType::CONSTANT_BUFFER, 
    bindDesc.BindPoint, bufferSize, bindDesc.Space);  // ← 添加 Space

// 第616行 - SRV
ShaderReflectionInfo->mTextureInfo.emplace_back(
    bindDesc.Name, ShaderResourceType::TEXTURE, 
    bindDesc.BindPoint, 0, bindDesc.Space);  // ← 添加 Space

// 第623行 - Sampler
ShaderReflectionInfo->mSamplerInfo.emplace_back(
    bindDesc.Name, ShaderResourceType::SAMPLER, 
    bindDesc.BindPoint, 0, bindDesc.Space);  // ← 添加 Space
```

---

### ✅ 步骤 3：重写 CreateRootSignatureByShaderReflection

**核心思路**：按 space 分组，每个 space 一个 table，简化为只用 b0, b1, b2...

```cpp
void D3D12RenderAPI::CreateRootSignatureByShaderReflection(Shader* shader)
{
    std::vector<CD3DX12_ROOT_PARAMETER> rootParams;
    std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
    std::vector<TableLayoutInfo> layout;
    
    // === 处理 CBV ===
    // 按 space 分组
    std::map<UINT /*space*/, std::vector<ShaderBindingInfo >> cbvBySpace;
    for (auto& cb : shader->mShaderBindingInfo.mBufferInfo) {
        cbvBySpace[cb.space].push_back(cb);
    }
    
    for (auto& [space, cbList] : cbvBySpace)
    {
        UINT rootIndex = rootParams.size();
        size_t rangeStart = ranges.size();
        
        // 为这个 space 的每个 CB 创建一个 range
        for (auto& cb : cbList)
        {
            ranges.emplace_back();
            ranges.back().Init(
                D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                1,                      // 每个 CB 1 个 descriptor
                cb.registerSlot,        // b0/b1/b2
                space,                  // 【关键】指定 space
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
            );
        }
        
        // 创建 root parameter
        rootParams.emplace_back();
        rootParams.back().InitAsDescriptorTable(
            cbList.size(), 
            &ranges[rangeStart]
        );
        
        // 记录布局信息
        layout.push_back({D3D12_DESCRIPTOR_RANGE_TYPE_CBV, space, rootIndex});
    }
    
    // === 处理 SRV（纹理）===
    std::map<UINT, std::vector<ShaderBindingInfo >> srvBySpace;
    for (auto& tex : shader->mShaderBindingInfo.mTextureInfo) {
        srvBySpace[tex.space].push_back(tex);
    }
    
    for (auto& [space, texList] : srvBySpace)
    {
        UINT rootIndex = rootParams.size();
        size_t rangeStart = ranges.size();
        
        for (auto& tex : texList)
        {
            ranges.emplace_back();
            ranges.back().Init(
                D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1,
                tex.registerSlot,       // t0/t1/t2/t3
                space,
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
            );
        }
        
        rootParams.emplace_back();
        rootParams.back().InitAsDescriptorTable(texList.size(), &ranges[rangeStart]);
        
        layout.push_back({D3D12_DESCRIPTOR_RANGE_TYPE_SRV, space, rootIndex});
    }
    
    // === 处理 Sampler（如果需要）===
    // ... 类似逻辑
    
    // 【重要】保存布局信息
    shader->mShaderBindingInfo.mRootParamLayout = layout;
    
    // 创建 Root Signature
    CD3DX12_STATIC_SAMPLER_DESC staticSampler(
        0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP
    );
    
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        static_cast<UINT>(rootParams.size()), rootParams.data(),
        1, &staticSampler,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );
    
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    ThrowIfFailed(D3D12SerializeRootSignature(
        &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        &serializedRootSig, &errorBlob
    ));
    
    ComPtr<ID3D12RootSignature> tempRootSignature;
    ThrowIfFailed(md3dDevice->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&tempRootSignature)
    ));
    
    shaderRootSignatureMap.try_emplace(shader->GetInstanceID(), tempRootSignature);
}
```

---
// 在分组后，对每个 space 的资源按 registerSlot 排序
for (auto& [space, cbList] : cbvBySpace)
{
    // 排序
    std::sort(cbList.begin(), cbList.end(), 
        [](const ShaderBindingInfo& a, const ShaderBindingInfo& b) {
            return a.registerSlot < b.registerSlot;
        });
    
    // 验证连续性（可选，调试用）
    for (int i = 0; i < cbList.size(); i++) {
        assert(cbList[i].registerSlot == i && "Register slots must be continuous!");
    }
    
    // 创建单个 range
    ranges.emplace_back();
    ranges.back().Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
        cbList.size(),  // 一次性分配
        0,              // 从 b0 开始
        space,
        0
    );
}







### ✅ 步骤 4：Root Signature 和 PSO 的绑定（无需修改）

**当前逻辑已经正确**（`D3D12PSOManager.cpp` 第28行）：

```cpp
psoDesc.pRootSignature = pso.rootSignature.Get();  // ✅ 已有
```

---

### ✅ 步骤 5：重写 RenderAPISetMaterial

```cpp
void D3D12RenderAPI::RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial)
{
    uint32_t matID = payloadSetMaterial.matId;
    uint32_t shaderID = payloadSetMaterial.shaderID;  // 需要从 Payload 获取
    
    TD3D12MaterialData& matData = m_DataMap[matID];
    Shader* shader = /* 获取 shader 指针 */;
    
    // 遍历 root signature 的布局
    for (auto& layoutInfo : shader->mShaderBindingInfo.mRootParamLayout)
    {
        UINT rootIndex = layoutInfo.rootParamIndex;
        UINT space = layoutInfo.space;
        
        if (layoutInfo.type == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
        {
            // 根据 space 选择数据源
            std::vector<TD3D12ConstantBuffer>* cbvSource = nullptr;
            
            if (space == 0) {
                cbvSource = &m_PerFrameData.constantBuffers;  // 需要创建这个结构
            } else if (space == 1) {
                cbvSource = &matData.mConstantBufferArray;
            } else if (space == 2) {
                cbvSource = &m_PerDrawData.constantBuffers;   // 需要创建这个结构
            }
            
            // 分配 descriptor table 空间
            TD3D12DescriptorHandle tableHandle = 
                D3D12DescManager::GetInstance()->GetFrameCbvSrvUavAllocator(cbvSource->size());
            
            // 拷贝 descriptors 到连续空间
            for (int i = 0; i < cbvSource->size(); i++)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE dest = {
                    tableHandle.cpuHandle.ptr + i * mCbvSrvUavDescriptorSize
                };
                md3dDevice->CopyDescriptorsSimple(
                    1, dest, 
                    (*cbvSource)[i].handleCBV.cpuHandle, 
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                );
            }
            
            // 绑定到对应的 root parameter index
            mCommandList->SetGraphicsRootDescriptorTable(rootIndex, tableHandle.gpuHandle);
        }
        else if (layoutInfo.type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
        {
            // 类似处理纹理
            std::vector<TD3D12TextureHander>* srvSource = nullptr;
            
            if (space == 0) {
                srvSource = &m_PerFrameData.textures;
            } else if (space == 1) {
                srvSource = &matData.mTextureBufferArray;
            }
            
            TD3D12DescriptorHandle tableHandle = 
                D3D12DescManager::GetInstance()->GetFrameCbvSrvUavAllocator(srvSource->size());
            
            for (int i = 0; i < srvSource->size(); i++)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE dest = {
                    tableHandle.cpuHandle.ptr + i * mCbvSrvUavDescriptorSize
                };
                
                TD3D12DescriptorHandle texSRVHandle = 
                    GetTextureSrvHanle((*srvSource)[i].textureID);
                
                md3dDevice->CopyDescriptorsSimple(
                    1, dest, texSRVHandle.cpuHandle, 
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                );
            }
            
            mCommandList->SetGraphicsRootDescriptorTable(rootIndex, tableHandle.gpuHandle);
        }
    }
}
```

---

## 📝 完整修改清单

### 文件 1: `Runtime/Core/PublicStruct.h`

```cpp
// 修正构造函数语法（第54行）
ShaderBindingInfo (const string& resourceName, ShaderResourceType type, 
                   int registerSlot, int size = 0, int space = 0)
    : resourceName(resourceName), type(type), registerSlot(registerSlot), 
      size(size), space(space)
{}

// 添加布局信息结构（第60行附近）
struct TableLayoutInfo
{
    D3D12_DESCRIPTOR_RANGE_TYPE type;
    UINT space;
    UINT rootParamIndex;
};

// 在 ShaderReflectionInfo 中添加（第70行附近）
vector<TableLayoutInfo> mRootParamLayout;
```

### 文件 2: `D3D12RenderAPI.cpp`

1. **修改 `CompileShaderStageAndGetReflection`**（第608, 616, 623行）：添加 `bindDesc.Space`
2. **重写 `CreateRootSignatureByShaderReflection`**（第390行开始）
3. **重写 `RenderAPISetMaterial`**（第1149行开始）
4. **添加 PerFrame/PerDraw 数据结构**（需要在类定义中添加）

### 文件 3: `Assets/Shader/SimpleTestShader.hlsl`

```hlsl
// 为纹理补充 space
Texture2D DiffuseTexture : register(t0, space1);
// ... 其他纹理
```

---

## ✨ 重构后的效果

```cpp
// 创建 Root Signature 时：
// Root Param #0: CBV Table (b0, space0) → PerFrame
// Root Param #1: CBV Table (b1, space1) → PerMaterial  
// Root Param #2: CBV Table (b2, space2) → PerDraw
// Root Param #3: SRV Table (t0~t3, space1) → Material Textures

// SetMaterial 时：
// 遍历 layout，根据 space 选择数据源，填充对应的 root parameter index
// rootIndex=0 → PerFrame 数据 → 绑定
// rootIndex=1 → PerMaterial 数据 → 绑定
// rootIndex=2 → PerDraw 数据 → 绑定
// rootIndex=3 → 材质纹理 → 绑定
```

需要我提供某个函数的完整实现代码吗？或者开始实际修改文件？