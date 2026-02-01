# Architecture Digest: BUILD_TOOLING
> Auto-generated. Focus: premake5.lua, WinBuildAndRun, Win-GenProjects, run_ai_analysis, msbuild, Tools/premake

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Key Files Index
- `[22]` **WinBuildAndRun.bat** *(Content Included)*
- `[21]` **premake5.lua** *(Content Included)*
- `[20]` **run_ai_analysis.bat** *(Content Included)*
- `[20]` **Win-GenProjects.bat** *(Content Included)*
- `[20]` **Runtime/Entry.cpp** *(Content Included)*
- `[12]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[12]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[8]` **Runtime/Graphics/GPUSceneManager.cpp** *(Content Included)*
- `[8]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[6]` **Runtime/Renderer/Renderer.h** *(Content Included)*
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h** *(Content Included)*
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h** *(Content Included)*
- `[4]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp** *(Content Included)*
- `[4]` **Runtime/Platforms/D3D12/d3dUtil.cpp** *(Content Included)*
- `[4]` **Assets/Shader/BlitShader.hlsl** *(Content Included)*
- `[4]` **Assets/Shader/GPUCulling.hlsl** *(Content Included)*
- `[4]` **Assets/Shader/SimpleTestShader.hlsl** *(Content Included)*
- `[4]` **Assets/Shader/StandardPBR.hlsl** *(Content Included)*
- `[4]` **Assets/Shader/StandardPBR_VertexPulling.hlsl** *(Content Included)*
- `[2]` **Editor/EditorGUIManager.h** *(Content Included)*
- `[2]` **Editor/EditorSettings.h**
- `[2]` **Runtime/CoreAssert.h**
- `[2]` **Runtime/EngineCore.h**
- `[2]` **Runtime/PreCompiledHeader.h**
- `[2]` **Runtime/Core/Game.h**
- `[2]` **Runtime/Core/InstanceID.h**
- `[2]` **Runtime/Core/Object.h**
- `[2]` **Runtime/Core/Profiler.h**
- `[2]` **Runtime/Core/PublicEnum.h**
- `[2]` **Runtime/Core/PublicStruct.h**
- `[2]` **Runtime/Core/ThreadSafeQueue.h**
- `[2]` **Runtime/GameObject/Camera.h**
- `[2]` **Runtime/GameObject/Component.h**
- `[2]` **Runtime/GameObject/ComponentType.h**
- `[2]` **Runtime/GameObject/GameObject.h**
- `[2]` **Runtime/GameObject/MeshFilter.h**
- `[2]` **Runtime/GameObject/MeshRenderer.h**
- `[2]` **Runtime/GameObject/MonoBehaviour.h**
- `[2]` **Runtime/GameObject/Transform.h**
- `[2]` **Runtime/Graphics/ComputeShader.h**
- `[2]` **Runtime/Graphics/GeometryManager.h**
- `[2]` **Runtime/Graphics/GPUBufferAllocator.h**
- `[2]` **Runtime/Graphics/GPUSceneManager.h**
- `[2]` **Runtime/Graphics/GPUTexture.h**
- `[2]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[2]` **Runtime/Graphics/IGPUResource.h**
- `[2]` **Runtime/Graphics/Material.h**
- `[2]` **Runtime/Graphics/Mesh.h**
- `[2]` **Runtime/Graphics/MeshUtils.h**
- `[2]` **Runtime/Graphics/RenderTexture.h**
- `[2]` **Runtime/Graphics/Shader.h**
- `[2]` **Runtime/Graphics/Texture.h**
- `[2]` **Runtime/Managers/Manager.h**
- `[2]` **Runtime/Managers/WindowManager.h**
- `[2]` **Runtime/MaterialLibrary/MaterialArchetypeRegistry.h**
- `[2]` **Runtime/MaterialLibrary/MaterialInstance.h**
- `[2]` **Runtime/MaterialLibrary/MaterialLayout.h**
- `[2]` **Runtime/MaterialLibrary/StandardPBR.h**
- `[2]` **Runtime/Math/AABB.h**
- `[2]` **Runtime/Math/Frustum.h**

## Evidence & Implementation Details

### File: `WinBuildAndRun.bat`
```bat
    
    :: 快速检查（只在必要时编译）
    msbuild TinyEngine.sln /p:Configuration=Debug /p:Platform=x64 /v:minimal /nologo
    
    if !ERRORLEVEL! == 0 (
        echo Starting Editor...
        
        :: 关键修改：切换到 exe 所在目录再运行
        cd "%EXE_DIR%"
        "EngineCore.exe"
        
        :: 返回原始目录
        cd "%ORIGINAL_DIR%"
    ) else (
        echo Build failed 1!
        pause
    )
) else (
    echo Executable not found. Full build required...
    msbuild TinyEngine.sln /p:Configuration=Debug /p:Platform=x64
    
    if !ERRORLEVEL! == 0 (
        echo Build successful! Starting TinyEngine...
        
        :: 关键修改：切换到 exe 所在目录再运行
        cd "%EXE_DIR%"
        "EngineCore.exe"
        
        :: 返回原始目录
        cd "%ORIGINAL_DIR%"
    ) else (
        echo Build failed 2!
        pause
    )
)

pause
```

### File: `premake5.lua`
```lua
 -- 根据不同的生成器设置项目目录
if _ACTION == "vs2022" then
    projectdir = "Projects/Windows/Visual Studio 2022"
elseif _ACTION == "vs2019" then
    projectdir = "Projects/Windows/Visual Studio 2019"
elseif _ACTION == "xcode4" then
    projectdir = "Projects/macOS/Xcode"
elseif _ACTION == "gmake2" then
    if os.host() == "linux" then
        projectdir = "Projects/Linux/Makefiles"
    elseif os.host() == "macosx" then
        projectdir = "Projects/macOS/Makefiles"
    else
        projectdir = "Projects/Unix/Makefiles"
    end
else
    -- 默认情况
    projectdir = "Projects/Generated"
end

workspace "TinyEngine"
	architecture "x64"
	startproject "EngineCore"
	location (projectdir)

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
```

### File: `run_ai_analysis.bat`
```bat

echo [2/3] Generating Architecture Digests (Python)...
python _ai\ai_digest.py
if errorlevel 1 (
    echo Error running Python script. Make sure python is installed.
    pause
    exit /b
)

echo [3/3] Preparing Context for AI...

REM 生成一个“聚合文件”或者提示词模板
set PROMPT_FILE=_ai\PROMPT_ARCH_REVIEW.txt
(
echo I am providing you with the "Architecture Digests" of my current C++ Renderer.
echo These files contain the key structs, classes, and flow logic for each subsystem.
echo.
echo Please review the following contexts:
echo [1] @_ai/DIGEST_RENDER_FLOW.md (How we execute a frame)
echo [2] @_ai/DIGEST_MATERIAL.md (How we handle shaders/data)
echo [3] @_ai/DIGEST_RESOURCE.md (How we handle memory/barriers)
echo [4] @_ai/DIGEST_RHI.md (Low level abstraction)
echo.
echo Based strictly on this evidence:
echo 1. Describe my current architecture style (e.g., Immediate mode vs FrameGraph, Bindless vs Slot-based).
echo 2. Identify the top 3 bottlenecks that prevent this from being a "Modern Renderer".
echo 3. Propose a specific refactoring roadmap for the Material System to support [Insert Goal, e.g., GPU-Driven].
) > "%PROMPT_FILE%"

type "%PROMPT_FILE%" | clip

echo.
echo ========================================================
echo DONE!
echo.
echo 1. Digests are in: root/_ai/
echo 2. A prompt has been COPIED to your clipboard.
echo.
echo Just open Cursor/Chat and PASTE (Ctrl+V).
echo ========================================================
pause
```

### File: `Win-GenProjects.bat`
```bat
@echo off

call Tools\premake\premake5.exe vs2022
PAUSE 

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

### File: `Assets/Shader/StandardPBR.hlsl`
```hlsl
// 纹理资源
//Texture2D g_Textures[1024] : register(t0, space0);
Texture2D DiffuseTexture : register(t0, space0);

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

### File: `Editor/EditorGUIManager.h`
```cpp
#include "GameObject/GameObject.h"

namespace EngineEditor
{
    using GameObject = EngineCore::GameObject;
    class EditorPanel;
    class EditorGUIManager
    {
    public:
    // 对象不允许纯虚函数，所以要用指针，因为是指针，所以*sInstance表示对象，外面返回引用&就行。
        static EngineEditor::EditorGUIManager* s_Instance;
        static EngineEditor::EditorGUIManager* GetInstance(){ return (s_Instance);};
        virtual ~EditorGUIManager();
        
        static void Create();
        static void OnDestory();
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        // Update不需要实现， 因为是一个纯Payload，无任何信息/
        void Update();
        // 录入指令部分，不需要有具体RenderAPI实现。
        void Render();
        void InitPanel();

        inline void SetCurrentSelected(GameObject* go) 
        { 
            currentSelected = go; 
        };
        inline GameObject* GetCurrentSelected()
        { 
            return currentSelected;
        };
    private:
        std::vector<EditorPanel*> mPanelList;
        GameObject* currentSelected = nullptr;
    };


}
```