# Architecture Digest: SHADER_LIB
> Auto-generated. Focus: Assets/Shader, Assets/Shader/include, hlsl, glsl, include, Lighting, Surface, PBR

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- Shader库是GPU驱动渲染的核心资产。
- 关注通用Shader结构、CBuffer布局与Pass一致性。

## Key Files Index
- `[58]` **Assets/Shader/StandardPBR.hlsl** *(Content Included)*
- `[58]` **Assets/Shader/StandardPBR_VertexPulling.hlsl** *(Content Included)*
- `[52]` **Assets/Shader/include/Core.hlsl** *(Content Included)*
- `[38]` **Assets/Shader/SimpleTestShader.hlsl** *(Content Included)*
- `[35]` **Assets/Shader/BlitShader.hlsl** *(Content Included)*
- `[35]` **Assets/Shader/GPUCulling.hlsl** *(Content Included)*
- `[24]` **Runtime/MaterialLibrary/StandardPBR.h** *(Content Included)*
- `[22]` **Runtime/Entry.cpp** *(Content Included)*
- `[22]` **Runtime/MaterialLibrary/StandardPBR.cpp** *(Content Included)*
- `[17]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[17]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[14]` **Runtime/Graphics/GPUSceneManager.cpp** *(Content Included)*
- `[14]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[11]` **Runtime/Renderer/Renderer.h** *(Content Included)*
- `[11]` **Runtime/Platforms/D3D12/d3dUtil.h** *(Content Included)*
- `[10]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[10]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp** *(Content Included)*
- `[9]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h** *(Content Included)*
- `[9]` **Runtime/Platforms/D3D12/d3dUtil.cpp** *(Content Included)*
- `[8]` **Runtime/MaterialLibrary/MaterialLayout.h** *(Content Included)*
- `[8]` **Runtime/Serialization/DDSTextureLoader.h**
- `[8]` **Runtime/Platforms/D3D12/D3D12RootSignature.h**
- `[7]` **Runtime/PreCompiledHeader.h**
- `[7]` **Runtime/Core/Profiler.h**
- `[7]` **Runtime/Core/PublicStruct.h**
- `[7]` **Runtime/GameObject/Camera.h**
- `[7]` **Runtime/GameObject/GameObject.h**
- `[7]` **Runtime/GameObject/MeshFilter.h**
- `[7]` **Runtime/GameObject/MeshRenderer.h**
- `[7]` **Runtime/Graphics/ComputeShader.h**
- `[7]` **Runtime/Graphics/GPUBufferAllocator.h**
- `[7]` **Runtime/Graphics/GPUSceneManager.h**
- `[7]` **Runtime/Graphics/Material.h**
- `[7]` **Runtime/Graphics/Mesh.h**
- `[7]` **Runtime/Math/AABB.h**
- `[7]` **Runtime/Math/Math.h**
- `[7]` **Runtime/Renderer/BatchManager.h**
- `[7]` **Runtime/Renderer/RenderAPI.h**
- `[7]` **Runtime/Renderer/RenderCommand.h**
- `[7]` **Runtime/Renderer/RenderSorter.h**
- `[7]` **Runtime/Resources/ResourceManager.h**
- `[7]` **Runtime/Scene/BistroSceneLoader.h**
- `[7]` **Runtime/Scene/Scene.h**
- `[7]` **Runtime/Scene/SceneManager.h**
- `[7]` **Runtime/Serialization/MaterialLoader.h**
- `[7]` **Runtime/Serialization/MeshLoader.h**
- `[7]` **Runtime/Serialization/SceneLoader.h**
- `[7]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[7]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[7]` **Editor/Panel/EditorMainBar.h**
- `[6]` **Runtime/GameObject/Transform.h**
- `[6]` **Runtime/Graphics/MeshUtils.h**
- `[6]` **Runtime/Graphics/Shader.h**
- `[6]` **Runtime/MaterialLibrary/MaterialArchytype.h**
- `[6]` **Runtime/Renderer/RenderEngine.h**
- `[6]` **Runtime/Renderer/RenderStruct.h**
- `[6]` **Runtime/Serialization/StreamHelper.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h**

## Evidence & Implementation Details

### File: `Assets/Shader/StandardPBR.hlsl`
```hlsl

// 采样器
SamplerState LinearSampler : register(s0, space0);
SamplerState PointSampler : register(s1, space0);
SamplerState AnisotropicSampler : register(s2,space0);
SamplerComparisonState ShadowSampler : register(s3, space0);

// 顶点着色器输入
struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};
```

### File: `Assets/Shader/StandardPBR_VertexPulling.hlsl`
```hlsl

// 采样器
SamplerState LinearSampler : register(s0, space0);
SamplerState PointSampler : register(s1, space0);
SamplerState AnisotropicSampler : register(s2,space0);
SamplerComparisonState ShadowSampler : register(s3, space0);

// 顶点着色器输入
struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};
```

### File: `Assets/Shader/include/Core.hlsl`
```hlsl
#define CORE_HLSLI

cbuffer DrawIndices : register(b0, space0)
{
    uint g_InstanceBaseOffset;
}
```
...
```hlsl
}

cbuffer PerPassData : register(b2, space0)
{
    float3 CameraPosition;
    float4x4 ViewMatrix; 
    float4x4 ProjectionMatrix;
}
```
...
```hlsl
};

struct Vertex
{
    float3 Position;
    float3 Normal;
    float2 TexCoord;
};
```
...
```hlsl


struct PerMaterialData
{
    float4 DiffuseColor;
    float4 SpecularColor;
    float Roughness;
    float Metallic;
    float2 TilingFactor;
    uint DiffuseTextureIndex;
};
```

### File: `Assets/Shader/SimpleTestShader.hlsl`
```hlsl

// 纹理资源
Texture2D g_Textures[1024] : register(t0, space0);


// 采样器
SamplerState LinearSampler : register(s0, space0);
SamplerState PointSampler : register(s1, space0);
SamplerState AnisotropicSampler : register(s2,space0);
SamplerComparisonState ShadowSampler : register(s3, space0);

// 顶点着色器输入
struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};
```

### File: `Assets/Shader/BlitShader.hlsl`
```hlsl

// BlitShader.hlsl
Texture2D SrcTexture : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

// 使用传统的顶点输入
struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;  
    float2 TexCoord : TEXCOORD0;
};
```

### File: `Assets/Shader/GPUCulling.hlsl`
```hlsl
// ==========================================

struct RenderProxy
{
    uint batchID;
};
```
...
```hlsl
// 2. 常量缓冲：视锥体平面和物体总数
// register(b0) -> 对应 C++ 的 CBV
cbuffer CullingParams : register(b0)
{
    // 视锥体的6个面：Left, Right, Bottom, Top, Near, Far
    // 这里的 float4 存储平面方程: Ax + By + Cz + D = 0
    // xyz 为法线(指向视锥体内侧), w 为距离 D
    float4 g_FrustumPlanes[6]; 
    
    // 需要剔除的实例总数
    uint g_TotalInstanceCount; 
};
```
...
```hlsl

// // 1. AABB 包围盒定义 (对应 C++ 结构体)
struct AABB
{
    float3 Min; // AABB 最小点
    float3 Max; // AABB 最大点
    float2 Padding;
};
```
...
```hlsl
    {
        uint instanceCount;
        InterlockedAdd(g_IndirectDrawCallArgs[batchID].InstanceCount, 1, instanceCount);
        // 执行后：InstanceCount 原子 +1；instanceCount 是加之前的值

        uint indexStart = g_IndirectDrawCallArgs[batchID].StartInstanceLocation;
        uint currentIndex = indexStart + instanceCount;
        // 4. 如果可见，将索引追加到输出列表
        // AppendStructuredBuffer 会自动处理原子计数
        //g_VisibleInstanceIndices.Append(instanceIndex);
        g_VisibleInstanceIndices[currentIndex] = instanceIndex;
    }

}
```

### File: `Runtime/MaterialLibrary/StandardPBR.h`
```cpp
#include "PublicEnum.h"

namespace Mat::StandardPBR
{
    using EngineCore::MaterialLayout;
    using EngineCore::ShaderVariableType;


    inline constexpr const char* ArchytypeName =  "Material.PBR";

    inline static uint64_t GetArchetypeID()
    {
        return 0;
    }

    inline MaterialLayout GetMaterialLayout()
    {   
        MaterialLayout materialLayout;
        materialLayout.AddProp("DiffuseColor", ShaderVariableType::VECTOR4, 16);
            
        // float4 SpecularColor
        materialLayout.AddProp("SpecularColor", ShaderVariableType::VECTOR4, 16);
        // float Roughness
        materialLayout.AddProp("Roughness", ShaderVariableType::FLOAT, 4);
        
        // float Metallic
        materialLayout.AddProp("Metallic", ShaderVariableType::FLOAT, 4);
        // float2 TilingFactor (8 bytes)
        materialLayout.AddProp("TilingFactor", ShaderVariableType::VECTOR2, 8);
        
        materialLayout.AddProp("DiffuseTextureIndex", ShaderVariableType::FLOAT, 4);
        
        materialLayout.AddProp("PaddingLast", ShaderVariableType::VECTOR3, 12);
        
        return materialLayout;
    }    
};
```

### File: `Runtime/Renderer/Renderer.h`
```cpp


namespace EngineCore
{
    class Renderer : public Manager<Renderer>
    {
    public:
        Renderer(): mRenderThread(&Renderer::RenderThreadMain, this), mRunning(true), mPerFrameData{}{};
        ~Renderer();
        static void Create();

        void BeginFrame();
        void Render(RenderContext& context);
        void EndFrame();

        void DrawIndexed(uint32_t vaoID, int count);
        void ResizeWindow(int width, int height);
        void OnDrawGUI();
        void SetPerDrawData(const PerDrawHandle& perDrawHandle);
        void DrawIndexedInstanced(Mesh* mesh, int count, const PerDrawHandle& perDrawHandle);
        void SetPerFrameData(UINT perFrameBufferID);
        void SetPerPassData(UINT perPassBufferID);
        
        void SetRenderState(const Material* mat, const RenderPassInfo &passinfo);

        void SetMaterialData(Material* mat);

        void ConfigureRenderTarget(const RenderPassInfo& passInfo);

        void SetMeshData(Mesh* meshFilter);

        void SetViewPort(const Vector2& viewportStartXY, const Vector2& viewportEndXY);
        // todo: complete this..
        void SetSissorRect(const Vector2& viewportStartXY, const Vector2& viewportEndXY);

        void ProcessDrawCommand(const DrawCommand& cmd);

        void CopyBufferRegion(const Payload_CopyBufferRegion& copyCmd);

        void DispatchComputeShader(const Payload_DispatchComputeShader& dispatchCmd);
        
        void SetResourceState(IGPUResource* resource, BufferResourceState state);
        
        void SetBindlessMat(Material* mat);
        void SetBindLessMeshIB(uint32_t id);
        
        void DrawIndirect(Payload_DrawIndirect payload);
        
        void RenderThreadMain() 
        {
            while (mRunning.load(std::memory_order_acquire) == true) 
            {
                PROFILER_ZONE("RenderThread::RenderLoop");

                PROFILER_EVENT_BEGIN("RenderThread::WaitForSignalFromMainThread");
                CpuEvent::MainThreadSubmited().Wait();
                PROFILER_EVENT_END("RenderThread::WaitForSignalFromMainThread");

                RenderAPI::GetInstance()->RenderAPIBeginFrame();
                DrawCommand cmd;

                PROFILER_EVENT_BEGIN("RenderThread::ProcessDrawComand");
                while(mRenderBuffer.PopBlocking(cmd))
                {
                    if (cmd.op == RenderOp::kEndFrame) break;
                    ProcessDrawCommand(cmd);
                }
                PROFILER_EVENT_END("RenderThread::ProcessDrawComand");
                // later do Gpu Fence...
                RenderAPI::GetInstance()->RenderAPISubmit();

#ifdef EDITOR          
                PROFILER_EVENT_BEGIN("RenderThread::ProcessEditorGUI");
                if (hasDrawGUI)
                {
                    EngineEditor::EditorGUIManager::GetInstance()->BeginFrame();
                    EngineEditor::EditorGUIManager::GetInstance()->Render();
                    EngineEditor::EditorGUIManager::GetInstance()->EndFrame();
                    hasDrawGUI = false;
                }
```
...
```cpp


                RenderAPI::GetInstance()->RenderAPIPresentFrame();

                CpuEvent::RenderThreadSubmited().Signal();

                if (hasResize)
                {
                    RenderAPI::GetInstance()->RenderAPIWindowResize(pendingResize);
                    hasResize = false;
                    pendingResize = { 0,0 };
                }
```
...
```cpp
        PerPassData_Forward mPerPassData_Forward;

        void FlushPerFrameData();
        void FlushPerPassData(const RenderContext& context);
        void CreatePerFrameData();
        void CreatePerPassForwardData();

    };
}
```

### File: `Runtime/Platforms/D3D12/d3dUtil.h`
```cpp
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

/*
#if defined(_DEBUG)
    #ifndef Assert
    #define Assert(x, description)                                  \
    {                                                               \
```
...
```cpp
                    else if(result == Debug::AssertBreak)           \
        {                                                           \
            __debugbreak();                                         \
        }                                                           \
        }                                                           \
    }
    #endif
#else
    #ifndef Assert
    #define Assert(x, description) 
    #endif
#endif 		
    */

class d3dUtil
{
```
...
```cpp
    static bool IsKeyDown(int vkeyCode);

    static std::string ToString(HRESULT hr);

    static UINT CalcConstantBufferByteSize(UINT byteSize)
    {
        // Constant buffers must be a multiple of the minimum hardware
        // allocation size (usually 256 bytes).  So round up to nearest
        // multiple of 256.  We do this by adding 255 and then masking off
        // the lower 2 bytes which store all bits < 256.
        // Example: Suppose byteSize = 300.
        // (300 + 255) & ~255
        // 555 & ~255
        // 0x022B & ~0x00ff
        // 0x022B & 0xff00
        // 0x0200
        // 512
        return (byteSize + 255) & ~255;
    }
```
...
```cpp
		const std::string& target);

    static D3D12_RESOURCE_DIMENSION GetFBOD3D12Dimesnsion(const EngineCore::TextureDimension& dimension);
    static DXGI_FORMAT GetFBOD3D12Format(const EngineCore::TextureFormat& format);
};

class DxException
{
```
...
```cpp
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};


struct D3D12DrawCommand
{
```

### File: `Runtime/Platforms/D3D12/D3D12ShaderUtils.h`
```cpp
#include "d3dUtil.h"

namespace EngineCore
{
    class D3D12ShaderUtils
    {
    public:
        static bool CompileShaderAndGetReflection(const string& path, Shader* shader);
        static bool CompileShaderStageAndGetReflection(const string& path, string entryPoint, string target, Shader* shader, ShaderStageType type, Microsoft::WRL::ComPtr<ID3DBlob>& blob);
        static bool D3D12ShaderUtils::CompileComputeShaderAndGetReflection(const string& path, ComputeShader* csShader);
        
        static Microsoft::WRL::ComPtr<ID3DBlob> GetVSBlob(uint32_t shaderID)
        {
            ASSERT(vsBlobMap.count(shaderID) > 0);
            return vsBlobMap[shaderID];
        }

        static Microsoft::WRL::ComPtr<ID3DBlob> GetPSBlob(uint32_t shaderID)
        {
            ASSERT(psBlobMap.count(shaderID) > 0);
            return psBlobMap[shaderID];
        }

        static Microsoft::WRL::ComPtr<ID3DBlob> GetCSBlob(uint32_t shaderID)
        {
            ASSERT(csBlobMap.count(shaderID) > 0);
            return csBlobMap[shaderID];
        }
        static unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3DBlob>> vsBlobMap;
        static unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3DBlob>> psBlobMap;
        static unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3DBlob>> csBlobMap;
    };
}
```

### File: `Runtime/MaterialLibrary/MaterialLayout.h`
```cpp
#include "Assert.h"

namespace EngineCore
{
    // 用来存
    struct TextureAssetInfo
    {
        // 这个地方直接记录
        char name[64];
        uint64_t ASSETID;
    };

    struct MaterialPropertyLayout
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
        ShaderVariableType type;
    };

    class MaterialLayout
    {
    public:

        void AddProp(const std::string& name, ShaderVariableType type, uint32_t size)
        {
            MaterialPropertyLayout prop;
            prop.name = name;
            prop.type = type;
            prop.size = size;
            prop.offset = currentOffset;
            layout.m_PropertyLayout[name] = prop;
            
            currentOffset += size;
        }
        // temp;
        // 【新增】硬编码构建一个标准布局，模拟未来的 JSON 加载结果
        // 对应 Shader/SimpleTestShader.hlsl 里的 cbuffer 结构
        static MaterialLayout GetDefaultPBRLayout()
        {

            // --- Chunk 0 ---
            // float4 DiffuseColor
            AddProp("DiffuseColor", ShaderVariableType::VECTOR4, 16);
            
            // float4 SpecularColor
            AddProp("SpecularColor", ShaderVariableType::VECTOR4, 16);

            // float Roughness
            AddProp("Roughness", ShaderVariableType::FLOAT, 4);
            
            // float Metallic
            AddProp("Metallic", ShaderVariableType::FLOAT, 4);

            // float2 TilingFactor (8 bytes)
            AddProp("TilingFactor", ShaderVariableType::VECTOR2, 8);
            
            AddProp("DiffuseTextureIndex", ShaderVariableType::FLOAT, 4);
            
            AddProp("PaddingLast", ShaderVariableType::VECTOR3, 12);

            // 此时 offset = 16+16+4+4+8 = 48 bytes
            // 还需要补齐到 16 字节对齐吗？HLSL cbuffer 是 16 字节对齐的
            // 目前 48 刚好是 16 的倍数，完美。
            
            layout.m_TotalSize = currentOffset;
            return layout;
        }

        uint32_t GetPropertyOffset(const std::string& name)
        {
            ASSERT(m_PropertyLayout.count(name) > 0);
            return m_PropertyLayout[name].offset;
        };

        uint32_t GetSize(){ return m_TotalSize;}

    private:
        std::unordered_map<std::string, MaterialPropertyLayout> m_PropertyLayout;
        uint32_t m_TotalSize = 0;
```