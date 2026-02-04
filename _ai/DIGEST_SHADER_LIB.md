# Architecture Digest: SHADER_LIB
> Auto-generated. Focus: Assets/Shader, Assets/Shader/include, hlsl, include, Lighting, Surface, PBR

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
- `[23]` **Runtime/MaterialLibrary/StandardPBR.cpp** *(Content Included)*
- `[22]` **Runtime/Entry.cpp** *(Content Included)*
- `[17]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[17]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[14]` **Runtime/Graphics/GPUSceneManager.cpp** *(Content Included)*
- `[14]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[11]` **Runtime/Renderer/Renderer.h** *(Content Included)*
- `[11]` **Runtime/Platforms/D3D12/d3dUtil.h** *(Content Included)*
- `[10]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[10]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp** *(Content Included)*
- `[9]` **Runtime/Serialization/MaterialLoader.h** *(Content Included)*
- `[9]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h** *(Content Included)*
- `[9]` **Runtime/Platforms/D3D12/d3dUtil.cpp** *(Content Included)*
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
- `[7]` **Runtime/Resources/ResourceManager.cpp**
- `[7]` **Runtime/Resources/ResourceManager.h**
- `[7]` **Runtime/Scene/BistroSceneLoader.h**
- `[7]` **Runtime/Scene/Scene.h**
- `[7]` **Runtime/Scene/SceneManager.h**
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
- `[6]` **Runtime/MaterialLibrary/MaterialLayout.h**
- `[6]` **Runtime/Renderer/RenderEngine.h**
- `[6]` **Runtime/Renderer/RenderStruct.h**
- `[6]` **Runtime/Serialization/StreamHelper.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h**

## Evidence & Implementation Details

### File: `Assets/Shader/StandardPBR.hlsl`
```hlsl
// 纹理资源
//Texture2D g_Textures[1024] : register(t0, space0);
Texture2D DiffuseTexture : register(t0, space0);
Texture2D NormalTexture : register(t1, space0);
Texture2D MetallicTexture : register(t2, space0);
Texture2D EmissiveTexture : register(t3, space0);

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
    float4 Tangent : TANGENT;
};
```
...
```hlsl
    //float3 color = direct + ambient + emissive;
    float3 color = direct + ambient;
    return float4(color, 1.0f);

    // // 采样纹理
    // float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    // float3 normalMap = NormalTexture.Sample(AnisotropicSampler, input.TexCoord).xyz;
    // float specularValue = SpecularTexture.Sample(PointSampler, input.TexCoord).r;
    
    // // 基础颜色
    // float3 albedo = diffuseColor.rgb * DiffuseColor.rgb;
    
    // // 简单光照计算
    // float3 normal = normalize(input.Normal);
    // float3 lightDir = normalize(-LightDirection);
    // float3 viewDir = normalize(CameraPosition - input.WorldPos);
    
    // // 漫反射
    // float NdotL = saturate(dot(normal, lightDir));
    // float3 diffuse = albedo * LightColor * LightIntensity * NdotL;
    
    // // 高光反射
    // float3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(saturate(dot(viewDir, reflectDir)), (1.0f - Roughness) * 128.0f);
    // float3 specular = SpecularColor.rgb * specularValue * spec * LightColor * LightIntensity;
    
    // // 环境光
    // float3 ambient = albedo * AmbientColor * AmbientStrength;
    
    // // 环境反射
    // float3 envReflect = reflect(-viewDir, normal);
    // float3 envColor = EnvironmentMap.Sample(LinearSampler, envReflect).rgb;
    // float3 fresnel = lerp(float3(0.04, 0.04, 0.04), albedo, Metallic);
    // float3 reflection = envColor * fresnel * (1.0f - Roughness);
    
    // // 最终颜色
    // float3 finalColor = ambient + diffuse + specular + reflection;
    
    // return float4(finalColor, diffuseColor.a);
}
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
    float4 Tangent : TANGENT;

};
```
...
```hlsl
    float2 uv = input.TexCoord;
    if(_FlipY > 0.1) uv.y = 1.0 - uv.y; 
    return pow(SrcTexture.Sample(LinearSampler, uv), 0.45);
}
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
#include "Core/PublicEnum.h"

namespace Mat::StandardPBR
{
    using EngineCore::MaterialLayout;
    using EngineCore::ShaderVariableType;


    inline static const string GetArchetypeName()
    {
        return "StandardPBR";
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
        
        materialLayout.AddProp("DiffuseTextureID", ShaderVariableType::FLOAT, 4);
        
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

### File: `Runtime/Serialization/MaterialLoader.h`
```cpp
#include <cstdio>

namespace EngineCore
{
    struct alignas(16) MetaMaterialHeader
    {
        bool     enable;
        uint64_t shaderID;
    };

    struct alignas(16) MetatextureDependency
    {
        char name[50];
        uint64_t ASSETID = 0;
    };

    struct alignas(16) MetaTextureToBindlessBlockIndex
    {
        char name[50];
        uint32_t offset;
    };

    class MaterialLoader: public IResourceLoader                       
    {
    public:
        virtual ~MaterialLoader() = default;
        virtual LoadResult Load(const std::string& relativePath) override
        {
            LoadResult result;
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream in(path, std::ios::binary);
            in.seekg(sizeof(AssetHeader));
            bool isBindless = false;
            StreamHelper::Read(in, isBindless);

            std::string archyTypeName;
            StreamHelper::ReadString(in, archyTypeName);
            if (archyTypeName.empty())
            {
                archyTypeName = "StandardPBR";
            }

            std::string shaderPath;
            if (archyTypeName == "StandardPBR")
            {
                shaderPath = RenderSettings::s_EnableVertexPulling
                    ? "Shader/StandardPBR_VertexPulling.hlsl"
                    : "Shader/StandardPBR.hlsl";
            }

            ASSERT(!shaderPath.empty());

            Material* mat = new Material();
            mat->SetAssetCreateMethod(AssetCreateMethod::Serialization);
            mat->SetPath(relativePath);
            mat->SetAssetID(AssetIDGenerator::NewFromFile(relativePath));
            mat->isBindLessMaterial = isBindless;
            mat->archyTypeName = archyTypeName;

            uint64_t assetPathID = AssetRegistry::GetInstance()->GetAssetIDFromPath(shaderPath);
            result.dependencyList.emplace_back
            (
                assetPathID,
                AssetType::Shader,
                [=](){
                    mat->mShader = ResourceHandle<Shader>(assetPathID);
                }
            );
            

            std::vector<MetatextureDependency> textureDependencyList;
            StreamHelper::ReadVector(in, textureDependencyList);
            for(auto& textureDependency : textureDependencyList)
            {
                std::string texName = std::string(textureDependency.name);
                mat->textureData[texName] = nullptr;
                result.dependencyList.emplace_back
                    (
                        textureDependency.ASSETID, 
                        AssetType::Texture2D,
```
...
```cpp

            std::vector<uint8_t> materialInstanceData;
            StreamHelper::ReadVector(in, materialInstanceData);
            mat->matInstance->SetInstanceData(materialInstanceData);

            if(mat->isBindLessMaterial)
            {
                std::vector<MetaTextureToBindlessBlockIndex> textureToBinlessOffsetList;
                StreamHelper::ReadVector(in, textureToBinlessOffsetList);
                for(auto& textureToBindLessBlockIndex : textureToBinlessOffsetList)
                {
                    mat->matInstance->GetLayout().textureToBlockIndexMap[textureToBindLessBlockIndex.name] = textureToBindLessBlockIndex.offset; 
                }
            }
```
...
```cpp
                    float alphaCutoff = 0.5f;
                    float transmissionFactor = 0.0f;
                    StreamHelper::Read(in, alphaModeRaw);
                    StreamHelper::Read(in, alphaCutoff);
                    StreamHelper::Read(in, transmissionFactor);
                    mat->alphaMode = static_cast<AlphaMode>(alphaModeRaw);
                    mat->alphaCutoff = alphaCutoff;
                    mat->transmissionFactor = transmissionFactor;
                }
            }

            result.resource = mat;
            return result;
        }

        void SaveMaterialToBin(const Material* mat, const std::string& relativePath, uint64_t id)
        {
```
...
```cpp
            header.assetID =id;
            header.type = 3;
            StreamHelper::Write(out, header);

            StreamHelper::Write(out, mat->isBindLessMaterial);
            StreamHelper::WriteString(out, mat->archyTypeName);
            std::vector<MetatextureDependency> textureDependencyList;
            for(auto& [key, value] : mat->textureHandleMap)
            {
                MetatextureDependency currTex;
                std::sprintf(currTex.name, key.c_str(), 50);
                std::snprintf(currTex.name, sizeof(currTex.name), "%s", key.c_str());
                currTex.ASSETID = value.GetAssetID();
                textureDependencyList.push_back(currTex);
            }
```
...
```cpp

            std::vector<uint8_t> materialInstanceData = mat->matInstance->GetInstanceData();
            StreamHelper::WriteVector(out, materialInstanceData);

            // 记录bindless插槽索引
            if(mat->isBindLessMaterial)
            {
                std::vector<MetaTextureToBindlessBlockIndex> textureToBinlessOffsetList;
                for(auto& [key, value] : mat->matInstance->GetLayout().textureToBlockIndexMap)
                {
                    MetaTextureToBindlessBlockIndex currTextureToIndex;
                    std::snprintf(currTextureToIndex.name, sizeof(currTextureToIndex.name), "%s", key.c_str());
                    currTextureToIndex.offset = value;
                    textureToBinlessOffsetList.push_back(currTextureToIndex);
                }
                StreamHelper::WriteVector(out, textureToBinlessOffsetList);
            }
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