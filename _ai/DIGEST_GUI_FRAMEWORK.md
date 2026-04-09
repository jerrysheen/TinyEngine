# Architecture Digest: GUI_FRAMEWORK
> Auto-generated. Focus: Runtime/External/Imgui, ImGui Core, ImGui Widgets, ImGui Tables, ImGui_ImplDX12, ImGui_ImplWin32, BeginFrame, EndFrame, RenderText, SliderAngle, DragInt, DragFloat, BeginTable, TableHeadersRow, ItemAdd, IsItemVisible

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作，并建立解耦的帧更新流（GameObject/Component、Scene、CPUScene/GPUScene、FrameContext多帧同步）。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。
- 针对更新链路重点追踪：Game::Update/Render/EndFrame -> SceneManager/Scene -> CPUScene -> GPUScene -> FrameContext。
- 重点识别NodeDirtyFlags、NodeDirtyPayload、PerFrameDirtyList、CopyOp等脏数据传播与跨帧同步结构。

## Understanding Notes
- ImGui GUI框架集成与扩展，包含核心、Widgets、Tables等所有ImGui模块。
- 关注编辑器UI实现、自定义控件、ImGui后端适配。

## Key Files Index
- `[25]` **Runtime/Graphics/RenderTexture.h** *(Content Included)*
- `[24]` **Runtime/Graphics/RenderTexture.cpp** *(Content Included)*
- `[20]` **Runtime/Entry.cpp** *(Content Included)*
- `[16]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[16]` **Runtime/Renderer/RenderBackend.h** *(Content Included)*
- `[10]` **Runtime/Renderer/RenderCommand.h** *(Content Included)*
- `[10]` **Editor/D3D12/D3D12EditorGUIManager.cpp** *(Content Included)*
- `[9]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[7]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[6]` **Runtime/Renderer/RenderBackend.cpp** *(Content Included)*
- `[6]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[6]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h** *(Content Included)*
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h** *(Content Included)*
- `[6]` **Editor/D3D12/D3D12EditorGUIManager.h** *(Content Included)*
- `[5]` **Runtime/Core/PublicStruct.h** *(Content Included)*
- `[5]` **Runtime/GameObject/Camera.h** *(Content Included)*
- `[5]` **Runtime/Renderer/RenderAPI.h** *(Content Included)*
- `[5]` **Runtime/Renderer/RenderEngine.h** *(Content Included)*
- `[5]` **Runtime/Renderer/RenderPipeLine/RenderPass.h** *(Content Included)*
- `[5]` **Editor/Panel/EditorInspectorPanel.cpp**
- `[4]` **Editor/EditorGUIManager.h**
- `[4]` **Runtime/Scene/GPUScene.h**
- `[4]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[4]` **Runtime/Platforms/D3D12/d3dUtil.cpp**
- `[4]` **Assets/Shader/BlitShader.hlsl**
- `[4]` **Assets/Shader/GPUCulling.hlsl**
- `[4]` **Assets/Shader/IndirectDrawCallCombineComputeShader.hlsl**
- `[4]` **Assets/Shader/SimpleTestShader.hlsl**
- `[4]` **Assets/Shader/StandardPBR.hlsl**
- `[4]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[3]` **Runtime/Scene/CPUScene.h**
- `[3]` **Runtime/Scene/Scene.h**
- `[3]` **Runtime/Scene/SceneManager.h**
- `[3]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[3]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp**
- `[3]` **Runtime/Core/Concurrency/CpuEvent.h**
- `[2]` **Editor/EditorSettings.h**
- `[2]` **Runtime/CoreAssert.h**
- `[2]` **Runtime/EngineCore.h**
- `[2]` **Runtime/PreCompiledHeader.h**
- `[2]` **Runtime/Core/Game.h**
- `[2]` **Runtime/Core/InstanceID.h**
- `[2]` **Runtime/Core/Object.h**
- `[2]` **Runtime/Core/Profiler.h**
- `[2]` **Runtime/Core/PublicEnum.h**
- `[2]` **Runtime/Core/ThreadSafeQueue.h**
- `[2]` **Runtime/GameObject/Camera.cpp**
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
- `[2]` **Runtime/Graphics/GPUTexture.h**
- `[2]` **Runtime/Graphics/IGPUBufferAllocator.h**

## Evidence & Implementation Details

### File: `Runtime/Graphics/RenderTexture.h`
```cpp
namespace EngineCore
{
    class RenderTexture
    {
    public:
        RenderTexture() = default;
        RenderTexture(TextureDesc textureDesc);


        inline int GetWidth() { return textureDesc.width; };
        inline int GetHeight() { return textureDesc.height; };
    public:
        IGPUTexture*  textureBuffer;
        TextureDesc textureDesc;
    };
```

### File: `Runtime/Graphics/RenderTexture.cpp`
```cpp
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    RenderTexture::RenderTexture(TextureDesc textureDesc)
        :textureDesc(textureDesc)
    {
        textureBuffer = RenderAPI::GetInstance()->CreateRenderTexture(textureDesc);
    }
}
```

### File: `Runtime/Entry.cpp`
```cpp
{
    std::cout << "APP Runing!!!!" << std::endl;
    EngineCore::Game::GetInstance()->Launch();
    std::cout << "APP ShutDown!!!!" << std::endl;
    return 0;

}
```

### File: `Runtime/Renderer/RenderBackend.h`
```cpp
        static void Create();

        void BeginFrame();
        void EndFrame();

        void DrawIndexed(uint32_t vaoID, int count);
        void ResizeWindow(int width, int height);
        void OnDrawGUI();
        void SetPerDrawData(const PerDrawHandle& perDrawHandle);
        void DrawIndexedInstanced(Mesh* mesh, int count, const PerDrawHandle& perDrawHandle);
        void SetPerFrameData(UINT perFrameBufferID);
        void SetPerPassData(UINT perPassBufferID);
        void SetFrame(FrameTicket* frameTicket, uint32_t frameID);
        
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
        
        void UploadBufferStaged(const BufferAllocation& alloc, void* data, uint32_t size);


        void DrawIndirect(Payload_DrawIndirect payload);
        void FlushPerFrameData();
        void FlushPerPassData(const RenderContext& context);
        void CreatePerFrameData();
        void CreatePerPassForwardData();

        void RecycleStagedBuffer(const FrameTicket* ticket);
        void SubmitStagedBuffer(const FrameTicket* ticket);
        
        void RenderThreadMain() 
        {
            while (mRunning.load(std::memory_order_acquire) == true) 
            {
                PROFILER_ZONE("RenderThread::RenderLoop");

                // PROFILER_EVENT_BEGIN("RenderThread::WaitForSignalFromMainThread");
                // CpuEvent::MainThreadSubmited().Wait();
                // PROFILER_EVENT_END("RenderThread::WaitForSignalFromMainThread");


                PROFILER_EVENT_BEGIN("RenderThread::ProcessDrawComand");
                DrawCommand cmd;
                if (!mRenderBuffer.TryPop(cmd)) 
                {
                    mDataAvailableEvent.Wait();
                    continue;
                }

                bool hasBeginFrame = false;
                bool hasEndFrame = false;
                while (mRunning.load(std::memory_order_acquire) == true)
                {
                    if (cmd.op == RenderOp::kBeginFrame) hasBeginFrame = true;
                    if (cmd.op == RenderOp::kEndFrame)
                    {
                        hasEndFrame = true;
                        break;
                    }

                    ProcessDrawCommand(cmd);
```
...
```cpp
                if (hasDrawGUI)
                {
                    EngineEditor::EditorGUIManager::GetInstance()->EndFrame();
                    hasDrawGUI = false;
                }
                // ImGui DrawData 已消费完毕，通知主线程可以安全调用 ImGui::NewFrame()
                CpuEvent::GUIDataConsumed().Signal();
                PROFILER_EVENT_END("RenderThread::ProcessEditorGUI");
#endif


                RenderAPI::GetInstance()->RenderAPIPresentFrame();


                if (hasResize)
                {
```

### File: `Runtime/Renderer/RenderCommand.h`
```cpp
#include "Renderer/FrameTicket.h"

namespace EngineCore
{
    enum class RenderOp : uint8_t
    {
        kInvalid = 0,
        kBeginFrame = 1,
        kEndFrame = 2,
        kSetRenderState = 3,
        kSetVBIB = 4,
        kSetViewPort = 5,
        kSetSissorRect = 6,
        //kDrawIndexed = 7,
        kSetMaterial = 8,
        kConfigureRT = 9,
        kWindowResize = 10,
        kIssueEditorGUIDraw = 11,
        kSetPerDrawData = 12,
        kDrawInstanced = 13,
        kSetPerFrameData = 14,
        kSetPerPassData = 15,
        kCopyBufferRegion = 16,
        kDispatchComputeShader = 17,
        kSetBufferResourceState = 18,
        kDrawIndirect = 19,
        kSetBindlessMat = 20,
        kSetBindLessMeshIB = 21,
        kSetFrame = 22,
        kCopyBufferStaged = 23,
    };

    enum class DepthComparisonFunc : uint8_t
    {
        LESS = 0,
        LEQUAL = 1,
        EQUAL = 2,
        GREATEQUAL = 3,
        GREAT = 4
    };

    enum class CullMode : uint8_t
    {
        CULLOFF = 0,
        CULLBACK = 1,
        CULLFRONT = 2,
    };

    enum class BlendState : uint8_t
    {
        SRCALHPHA = 0,
        ONEMINUSSRCALPHA = 2,
        ONE = 3,
        ZERO = 4,
    };

    // 和材质相关，关联材质后可以做合批操作。
    struct MaterailRenderState
    {
        uint32_t shaderInstanceID = 0;
        RootSignatureKey rootSignatureKey;
        // depth stencil state:
        bool enableDepthTest = true;
        bool enableDepthWrite = true;
        DepthComparisonFunc depthComparisonFunc = DepthComparisonFunc::LEQUAL;
        bool isBindLessMaterial = false;
        // blend mode
        bool enableBlend = false;
        BlendState srcBlend = (BlendState)0;
        BlendState destBlend = (BlendState)0;
        uint32_t hashID = 0;
        uint32_t GetHash()
        {
            if(hashID != 0) return hashID;
            hashID = 0;
            HashCombine(hashID, shaderInstanceID);
            HashCombine(hashID, static_cast<uint32_t>(enableDepthTest));
            HashCombine(hashID, static_cast<uint32_t>(enableDepthWrite));
            HashCombine(hashID, static_cast<uint32_t>(depthComparisonFunc));
            HashCombine(hashID, static_cast<uint32_t>(enableBlend));
```
...
```cpp
    };

    struct PSODesc
    {
        MaterailRenderState matRenderState;

        TextureFormat colorAttachment = TextureFormat::EMPTY;
        TextureFormat depthAttachment = TextureFormat::EMPTY;
        uint32_t GetHash()
        {
            if(hashID != 0) return hashID;
            hashID = 0;
            HashCombine(hashID, matRenderState.shaderInstanceID);
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.enableDepthTest));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.depthComparisonFunc));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.enableDepthWrite));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.enableBlend));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.srcBlend));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.destBlend));
            HashCombine(hashID, static_cast<uint32_t>(colorAttachment));
            HashCombine(hashID, static_cast<uint32_t>(depthAttachment));
            return hashID;
        };
    private:
        uint32_t hashID = 0;
    };
```
...
```cpp
        float depthValue;
        ClearFlag flags;
        ClearValue() : colorValue(Vector3::Zero), depthValue(1.0f), flags(ClearFlag::None) {};
```
...
```cpp
    };

    struct Payload_BeginFrame { uint64_t frameIndex; };
```
...
```cpp

    // todo : mrt渲染，
    struct Payload_SetRenderState 
    {
        PSODesc psoDesc;
    };
```

### File: `Runtime/Platforms/D3D12/D3D12RenderAPI.h`
```cpp
    public:

        D3D12RenderAPI();
        ~D3D12RenderAPI(){};
```
...
```cpp
        
        // 渲染线程调用接口
        virtual void RenderAPIBeginFrame() override;
        virtual void RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT) override;
        virtual void RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand) override;
        virtual void RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial) override;
        virtual void RenderAPISetBindlessMat(Payload_SetBindlessMat payloadSetBindlessMat) override;
        virtual void RenderAPISetBindLessMeshIB() override;
        virtual void RenderAPISetRenderState(Payload_SetRenderState payloadSetRenderState) override;
        virtual void RenderAPISetSissorRect(Payload_SetSissorRect payloadSetSissorrect) override;
        virtual void RenderAPISetVBIB(Payload_SetVBIB payloadSetVBIB) override;
        virtual void RenderAPISetViewPort(Payload_SetViewPort payloadSetViewport) override;
        virtual void RenderAPIWindowResize(Payload_WindowResize payloadWindowResize) override;
        virtual void RenderAPISubmit() override;
        virtual void RenderAPIPresentFrame() override;
        virtual void RenderAPISetPerDrawData(Payload_SetPerDrawData setPerDrawData) override;
        virtual void RenderAPIDrawInstanceCmd(Payload_DrawInstancedCommand setDrawInstanceCmd) override;
        virtual void RenderAPISetPerPassData(Payload_SetPerPassData setPerPassData) override;
        virtual void RenderAPISetPerFrameData(Payload_SetPerFrameData setPerFrameData) override;
        virtual void RenderAPISetFrame(Payload_SetFrame setFrame) override;
        virtual void RenderAPICopyRegion(Payload_CopyBufferRegion copyBufferRegion) override;
        virtual void RenderAPIDispatchComputeShader(Payload_DispatchComputeShader dispatchComputeShader) override;
        virtual void RenderAPISetBufferResourceState(Payload_SetBufferResourceState bufferResourceState) override;
        virtual void RenderAPIExecuteIndirect(Payload_DrawIndirect drawIndirect) override;
        virtual void RenderAPICopyBufferStaged(Payload_CopyBufferStaged copyBufferStaged) override;
        
        virtual void CreateGlobalConstantBuffer(uint32_t enumID, uint32_t size) override;
        virtual RenderTexture* GetCurrentBackBuffer() override;

        virtual void SetGlobalDataImpl(uint32_t bufferID, uint32_t offset, uint32_t size, const void* value) override;

        TD3D12ConstantBuffer CreateConstantBuffer(uint32_t size);

        Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
        UINT mRtvDescriptorSize = 0;
        UINT mDsvDescriptorSize = 0;
        UINT mCbvSrvUavDescriptorSize = 0;
        const int MAX_FRAME_INFLIAGHT = 3;
        DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT mCurrBackBuffer = 0;
        static const int SwapChainBufferCount = 3;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
        // 给底层使用的具体资源
        D3D12Texture mBackBuffer[SwapChainBufferCount];
        // 一个壳，上层用，IGPUTexture = mBackBuffer
        RenderTexture mBackBufferProxyRenderTexture;
        D3D12Texture mBackBufferProxy;
        
        TD3D12Fence* mFrameFence;
        TD3D12Fence* mImediatelyFence;

        D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderAPI::CurrentBackBufferView()const
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(
                mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
                mCurrBackBuffer,
                mRtvDescriptorSize);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderAPI::DepthStencilView()const
        {
            return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
        }

        ID3D12Resource* D3D12RenderAPI::CurrentBackBuffer()const
        {
            return mBackBuffer[mCurrBackBuffer].m_Resource.Get();
        }

        void SignalFence(TD3D12Fence* mFence);
        void WaitForFence(TD3D12Fence* mFence);
        void WaitForRenderFinish(TD3D12Fence* mFence);
        virtual void WaitForGpuFinished() override { WaitForFence(mFrameFence); }

        ComPtr<ID3D12PipelineState> psoObj;
        ComPtr<ID3D12RootSignature> rootSignature;
        
        void ImmediatelyExecute(std::function<void(ComPtr<ID3D12GraphicsCommandList> cmdList)>&& function);
        
        virtual IGPUBuffer* CreateBuffer(const BufferDesc& desc, void* data) override;
```
...
```cpp
    private:

        bool InitDirect3D();
        void InitFence();
        void InitDescritorHeap();
        void InitCommandObject();
        void InitSwapChain();
        void InitRenderTarget();

        int GetNextVAOIndex();

        DXGI_FORMAT ConvertD3D12Format(TextureFormat format);
        inline bool IsCompressedFormat(TextureFormat format)
        {
            return format >= TextureFormat::DXT1 && format <= TextureFormat::BC7_SRGB;
        }
```
...
```cpp
                case TextureFormat::R8G8B8A8: return 4;
                case TextureFormat::D24S8: return 4;
                default: ASSERT(false);
            }
        }

        // RowPitch计算的是一行像素的字节数
        // 比如压缩图，4x4块会被压缩成 xxByte 比如8B
        inline uint32_t CalculateCompressedRowPitch(TextureFormat format, uint32_t width)
        {
```
...
```cpp
                    return blockWidth * 16;
                default:
                    ASSERT_MSG(false, "Not Implemented yet");
            }
        }

        // 计算整个切片/mipmap level的总字节数
        inline uint32_t CalculateCompressedSlicePitch(TextureFormat format, uint32_t width, uint32_t height)
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

### File: `Editor/D3D12/D3D12EditorGUIManager.h`
```cpp
    public:
        // 渲染线程beginFrame，重置CommandList
        virtual void BeginFrame() override;
        // 渲染线程EndFrame，提交指令
        virtual void EndFrame() override;
        static void ApplyDpiScale(float scale)
        {
            // 1) 缩放 ImGui Style
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(scale);

            // 2) 重新构建字体（把像素大小乘上 scale）
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->Clear();
            // 举例：原 16px 字体，按比例放大
            io.Fonts->AddFontFromFileTTF("c:/Windows/Fonts/segoeui.ttf", 16.0f * scale);
            io.Fonts->Build();

            // 3) 如果你不想重建字体，也可以简单用全局缩放(体验略差)
            // io.FontGlobalScale = scale; // 简易方案（字体不会变清晰度）
        }
```
...
```cpp
        std::vector<uint64_t> allocatorFenceValues;

		void InitForDirectX12();
		UINT GetNextAvailablePos();

    };
}
```

### File: `Runtime/Core/PublicStruct.h`
```cpp
#include "Graphics/RenderTexture.h"

namespace EngineCore
{



    // constantbuffer中的变量记录
    struct ShaderConstantInfo
    {
        string variableName;
        ShaderVariableType type;
        int bufferIndex;
        int size;
        int offset;
    };

    // shader中通过反射得到的资源名称、类型
    struct ShaderBindingInfo 
    {
        string resourceName;
        ShaderResourceType type;
        int registerSlot;              
        int size = 0;                  // 对CB有意义，其他资源可为0
        int space = 0;
        int bindCount = 1;             // 绑定数量，数组时 > 1
        ShaderBindingInfo (const string& resourceName, ShaderResourceType type, int registerSlot, int size, int space, int bindCount = 1)
            : resourceName(resourceName), type(type), registerSlot(registerSlot), size(size),
            space(space), bindCount(bindCount)
        {};
    };


    struct ShaderReflectionInfo
    {
        RootSignatureKey mRootSigKey;

        // todo: 确定这个地方是用vector还是直接单个对象
        ShaderStageType type;
        vector<ShaderBindingInfo > mConstantBufferInfo;
        vector<ShaderBindingInfo > mTextureInfo;
        vector<ShaderBindingInfo > mSamplerInfo;
        vector<ShaderBindingInfo > mUavInfo;

        ShaderReflectionInfo(){};


        // 定义偏移量常量 (方便修改)
        static const int BIT_OFFSET_CBV = 0;
        static const int BIT_OFFSET_SRV = 16;
        static const int BIT_OFFSET_UAV = 48;
        static const int BIT_OFFSET_SAMPLER = 56;
    };

    struct LightData
    {

    };


    // 前向声明，防止循环引用。
    class Transform;
    class MeshRenderer;
    class MeshFilter;

    struct RenderPacket
    {
        // 为了测试，先用直接塞数据的方式。
        uint64_t sortingKey = 0;
        AssetID meshID;
        AssetID materialID;
        uint32_t objectIndex;
        float distanToCamera = 0;
    };

    struct PerDrawHandle
    {
        uint8_t* destPtr;
        uint32_t offset;
        uint32_t size;
```
...
```cpp


    struct DrawRecord
    {
        Material* mat;
        Mesh* mesh;

        PerDrawHandle perDrawHandle;
        uint32_t instanceCount = 1;

        DrawRecord(Material* mat, Mesh* mesh)
            :mat(mat), mesh(mesh), perDrawHandle{0,0}, instanceCount(1) {}
        DrawRecord(Material* mat, Mesh* mesh, const PerDrawHandle& handle, uint32_t instCount = 1)
            :mat(mat), mesh(mesh), perDrawHandle(handle), instanceCount(instCount){}
    };
```
...
```cpp
    };

    class RenderPass;
    struct RenderPassAsset
    {
        vector<RenderPass*> renderPasses;
        inline void Clear()
        {
             for (RenderPass* pass : renderPasses) 
                delete pass;
        };
    };
```
...
```cpp
    };

    struct ContextFilterSettings
    {

    };
```
...
```cpp
    {
        uint32_t perObejectIndex = UINT32_MAX;
        inline bool isValid() const {return perObejectIndex != UINT32_MAX;}
```

### File: `Runtime/GameObject/Camera.h`
```cpp
#include "Graphics/RenderTexture.h"

namespace EngineCore
{
    class Camera : public Component
    {
    public:
        Camera() = default;
        Camera(GameObject* parent);
        virtual ~Camera() override {};
        float mFov;
        float mAspect;
        float mNear;
        float mFar; 
        float mWidth;
        float mHeight;
        Matrix4x4 mProjectionMatrix;
        Vector3 mLookAt;
        Matrix4x4 mViewMatrix;
        Frustum mFrustum;

        static ComponentType GetStaticType() { return ComponentType::Camera; };
        virtual ComponentType GetType() const override { return ComponentType::Camera; };
        void Update();
    public:
        RenderPassAsset mRenderPassAsset;
        // 这两个只是描述符， 没必要用指针
        RenderTexture* depthAttachment;
        RenderTexture* colorAttachment;
        void UpdateCameraMatrix();


        virtual const char* GetScriptName() const override { return "Camera"; }
    };

}
```

### File: `Runtime/Renderer/RenderAPI.h`
```cpp


namespace  EngineCore
{
    class RenderAPI
    {
    public:
        inline static RenderAPI* GetInstance()
        {
            if (s_Instance == nullptr) 
            {   
                Create();
            }
            return s_Instance.get();
        }
        static bool IsInitialized(){return s_Instance != nullptr;};
       
        static void Create();
        virtual void  CompileShader(const string& path, Shader* shader) = 0;
        virtual void  CompileComputeShader(const string& path, ComputeShader* csShader) = 0;
        

        virtual IGPUTexture* CreateTextureBuffer(unsigned char* data, const TextureDesc& textureDesc) = 0;
        virtual IGPUTexture* CreateRenderTexture(const TextureDesc& textureDesc) = 0;
        
        //virtual void GetOrCreatePSO(const Material& mat, const RenderPassInfo &passinfo) = 0;
        inline void AddRenderPassInfo(const RenderPassInfo& renderPassInfo){ mRenderPassInfoList.push_back(renderPassInfo); };
        inline void ClearRenderPassInfo(){ mRenderPassInfoList.clear(); };
        
        virtual void RenderAPIBeginFrame() = 0;
        virtual void RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT) = 0;
        virtual void RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand) = 0;
        virtual void RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial) = 0;
        virtual void RenderAPISetBindlessMat(Payload_SetBindlessMat payloadSetBindlessMat) = 0;
        virtual void RenderAPISetBindLessMeshIB() = 0;
        virtual void RenderAPISetRenderState(Payload_SetRenderState payloadSetRenderState) = 0;
        virtual void RenderAPISetSissorRect(Payload_SetSissorRect payloadSetSissorrect) = 0;
        virtual void RenderAPISetVBIB(Payload_SetVBIB payloadSetVBIB) = 0;
        virtual void RenderAPISetViewPort(Payload_SetViewPort payloadSetViewport) = 0;
        virtual void RenderAPISubmit() = 0;
        virtual void RenderAPIWindowResize(Payload_WindowResize payloadWindowResize) = 0;
        virtual void RenderAPIPresentFrame() = 0;
        virtual void RenderAPISetPerPassData(Payload_SetPerPassData setPerPassData) = 0;
        virtual void RenderAPISetPerFrameData(Payload_SetPerFrameData setPerFrameData) = 0;
        virtual void RenderAPISetFrame(Payload_SetFrame setFrame) = 0;
        virtual void RenderAPIExecuteIndirect(Payload_DrawIndirect drawIndirect) = 0;
        
        virtual void CreateGlobalConstantBuffer(uint32_t enumID, uint32_t size) = 0;
        
        virtual void RenderAPISetPerDrawData(Payload_SetPerDrawData setPerDrawData) = 0;
        virtual void RenderAPIDrawInstanceCmd(Payload_DrawInstancedCommand setDrawInstanceCmd) = 0;
        
        virtual IGPUBuffer* CreateBuffer(const BufferDesc& desc, void* data) = 0;
        virtual void UploadBuffer(IGPUBuffer* buffer, uint32_t offset, void* data, uint32_t size) = 0;
        virtual void RenderAPICopyRegion(Payload_CopyBufferRegion copyBufferRegion) = 0;
        virtual void RenderAPIDispatchComputeShader(Payload_DispatchComputeShader dispatchComputeShader) = 0;
        virtual void RenderAPISetBufferResourceState(Payload_SetBufferResourceState bufferResourceState) = 0;
        virtual void RenderAPICopyBufferStaged(Payload_CopyBufferStaged copyBufferStaged) = 0;
        virtual RenderTexture* GetCurrentBackBuffer() = 0;
        template<typename T>
        void SetGlobalValue(uint32_t bufferID, uint32_t offset, T* value)
        {
            uint32_t size = sizeof(T);
            SetGlobalDataImpl(bufferID, offset, size, static_cast<void*>(value));
        }
        virtual void WaitForGpuFinished() = 0;

        virtual uint64_t GetCurrentGPUCompletedFenceValue() = 0;
    public:
        static std::unique_ptr<RenderAPI> s_Instance;
    protected:
        vector<RenderPassInfo> mRenderPassInfoList;
    private:
        virtual void SetGlobalDataImpl(uint32_t bufferID, uint32_t offset, uint32_t size, const void* value) = 0;

    };
    
} // namespace  EngineCore

```

### File: `Runtime/Renderer/RenderEngine.h`
```cpp
    public:
        static RenderEngine* GetInstance(){return s_Instance.get();};
        static bool IsInitialized(){return s_Instance != nullptr;};
```
...
```cpp
        void BuildFrame();
        void WaitForFrameAvaliable(uint32_t frameID);
        void EndFrame();
        
        static void OnResize(int width, int height);
        static void OnDrawGUI();
        static void Destory();
        RenderEngine(){};
```
...
```cpp
        }

        inline uint32_t GetCurrentFrame(){ return mCurrentFrameID;}
```
...
```cpp
        uint32_t mCurrentFrameID = 0;

        void ComsumeDirtySceneRenderNode(const SceneDelta& delta);
    };
    
}
```

### File: `Runtime/Renderer/RenderPipeLine/RenderPass.h`
```cpp
#include "Graphics/RenderTexture.h"

namespace EngineCore
{
    class RenderPass
    {
    public:

        // init RenderPass, for example Create FBO..
        //virtual void Create(const RenderContext& context) = 0;
        virtual void Create() = 0;
        
        // Record Rendertarget in mRenderPassInfo..
        virtual void Configure(const RenderContext& context) = 0;
        
        // decide what content we need to draw, for example
        // in opaquepass we only need gameobject renderqueue less than 3000.
        virtual void Filter(const RenderContext& context) = 0;
        
        // first record drawInfo then sync data to RenderAPI, finally Submit to execute the
        // CPU Prepare Data.
        virtual void Prepare(RenderContext& context) = 0;

        // first record drawInfo then sync data to RenderAPI, finally Submit to execute the
        // Submit To RenderThread
        virtual void Execute(RenderContext& context) = 0;
        
        // sent current Data to Renderer-> RenderPassInfo
        virtual void Submit() = 0;

        inline void Clear() { mRenderPassInfo.Reset(); };


        // ignore null ptr since RenderAPI level will handle this problem
        inline void SetRenderTarget(RenderTexture* colorAttachment, RenderTexture* depthAttachment)
        {
            mRenderPassInfo.colorAttachment = colorAttachment;
            mRenderPassInfo.depthAttachment = depthAttachment;
        };

        inline void SetClearFlag(ClearFlag flag, const Vector3& colorValue = Vector3::Zero, float depthValue = 1.0)
        {
            mRenderPassInfo.clearFlag = flag;
            mRenderPassInfo.clearColorValue = colorValue;
            mRenderPassInfo.clearDepthValue = depthValue;
        };

        inline void SetViewPort(const Vector2& startPos, const Vector2& endPos)
        {
            mRenderPassInfo.viewportStartPos = startPos;
            mRenderPassInfo.viewportEndPos = endPos;
        }

        inline const RenderPassInfo& GetRenderPassInfo(){return mRenderPassInfo;};
        RootSigSlot mRootSigSlot;
    protected:
        void IssueRenderCommandCommon(const RenderPassInfo& passInfo,
                                      const std::vector<RenderBatch>& batches
                                      );
    public:
        string name;
        RenderPassInfo mRenderPassInfo;
    };
} // namespace EngineCore
```