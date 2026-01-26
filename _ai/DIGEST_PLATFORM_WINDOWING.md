# Architecture Digest: PLATFORM_WINDOWING
> Auto-generated. Focus: Runtime/Platforms, Window, Input, Platform, Win32, MessageLoop

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 窗口、输入与平台层抽象决定跨平台能力。
- 关注窗口创建、消息循环与输入采样接口。

## Key Files Index
- `[47]` **Runtime/Platforms/Windows/WindowManagerWindows.h** *(Content Included)*
- `[27]` **Runtime/Managers/WindowManager.h** *(Content Included)*
- `[25]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[24]` **Runtime/Platforms/D3D12/D3D12Struct.h** *(Content Included)*
- `[24]` **Runtime/Platforms/D3D12/d3dUtil.h** *(Content Included)*
- `[22]` **Runtime/Platforms/D3D12/D3D12Buffer.h** *(Content Included)*
- `[22]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h** *(Content Included)*
- `[22]` **Runtime/Platforms/D3D12/D3D12DescManager.h** *(Content Included)*
- `[22]` **Runtime/Platforms/D3D12/D3D12PSO.h** *(Content Included)*
- `[22]` **Runtime/Platforms/D3D12/D3D12RootSignature.h** *(Content Included)*
- `[22]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h** *(Content Included)*
- `[22]` **Runtime/Platforms/D3D12/D3D12Texture.h** *(Content Included)*
- `[22]` **Runtime/Platforms/D3D12/d3dx12.h** *(Content Included)*
- `[9]` **Runtime/PreCompiledHeader.h**
- `[7]` **Editor/EditorSettings.h**
- `[7]` **Runtime/Graphics/Mesh.h**
- `[6]` **Runtime/Renderer/RenderCommand.h**
- `[5]` **premake5.lua**
- `[5]` **Runtime/Renderer/RenderAPI.h**
- `[5]` **Runtime/Renderer/Renderer.h**
- `[5]` **Runtime/Settings/ProjectSettings.h**
- `[5]` **Assets/Shader/BlitShader.hlsl**
- `[5]` **Assets/Shader/GPUCulling.hlsl**
- `[5]` **Assets/Shader/SimpleTestShader.hlsl**
- `[5]` **Assets/Shader/StandardPBR.hlsl**
- `[5]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[4]` **Runtime/Graphics/Shader.h**
- `[4]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[4]` **Editor/D3D12/D3D12EditorGUIManager.h**
- `[3]` **Runtime/EngineCore.h**
- `[2]` **Editor/EditorGUIManager.h**
- `[2]` **Runtime/CoreAssert.h**
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
- `[2]` **Runtime/Graphics/MaterialData.h**
- `[2]` **Runtime/Graphics/MaterialInstance.h**
- `[2]` **Runtime/Graphics/MaterialLayout.h**
- `[2]` **Runtime/Graphics/MeshUtils.h**
- `[2]` **Runtime/Graphics/RenderTexture.h**

## Evidence & Implementation Details

### File: `Runtime/Platforms/Windows/WindowManagerWindows.h`
```cpp
namespace EngineCore
{
    class WindowManagerWindows : public WindowManager
    {
    public :
        static void Update();
        static void Create();
        virtual bool WindowShouldClose() override;
        virtual void OnResize() override;
        WindowManagerWindows();
        virtual void* GetWindow() override {return mWindow;}
        LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        void Show();
    private:
        void InitializeWindowsWindow();
        HWND mWindow;
    };
```

### File: `Runtime/Managers/WindowManager.h`
```cpp
namespace EngineCore
{
    class  WindowManager : public Manager<WindowManager>
    {
    public:
        static void Update();
        static void Create();
        virtual bool WindowShouldClose() = 0;
        virtual void OnResize() = 0;
        virtual void* GetWindow() = 0;
        inline int GetWidth() { return mWindowWidth; };
        inline int GetHeight() { return mWindowHeight; };
        std::pair<int, int> GetWindowSize()
        {
            return {mWindowWidth, mWindowHeight};
        }
        ~WindowManager() override {};
        WindowManager(){};
    protected:
        int mWindowWidth;
        int mWindowHeight;
        bool mMinimized = false;
        bool mResizing = false;
        bool mResized = false;
        bool mAppPaused = false;
		bool mMaximized = false;
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
        virtual void RenderAPISetVBIB(Payload_SetVBIB payloadSetVBIB) override;
        virtual void RenderAPISetViewPort(Payload_SetViewPort payloadSetViewport) override;
        virtual void RenderAPIWindowResize(Payload_WindowResize payloadWindowResize) override;
        virtual void RenderAPISubmit() override;
        virtual void RenderAPIPresentFrame() override;
        virtual void RenderAPISetPerDrawData(Payload_SetPerDrawData setPerDrawData) override;
        virtual void RenderAPIDrawInstanceCmd(Payload_DrawInstancedCommand setDrawInstanceCmd) override;
        virtual void RenderAPISetPerPassData(Payload_SetPerPassData setPerPassData) override;
        virtual void RenderAPISetPerFrameData(Payload_SetPerFrameData setPerFrameData) override;
        virtual void RenderAPICopyRegion(Payload_CopyBufferRegion copyBufferRegion) override;
        virtual void RenderAPIDispatchComputeShader(Payload_DispatchComputeShader dispatchComputeShader) override;
        virtual void RenderAPISetBufferResourceState(Payload_SetBufferResourceState bufferResourceState) override;
        virtual void RenderAPIExecuteIndirect(Payload_DrawIndirect drawIndirect) override;

        
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
```
...
```cpp
        }

        void SignalFence(TD3D12Fence* mFence);
        void WaitForFence(TD3D12Fence* mFence);
        void WaitForRenderFinish(TD3D12Fence* mFence);
        virtual void WaitForGpuFinished() override { WaitForFence(mFrameFence); }
```
...
```cpp
        ComPtr<ID3D12RootSignature> rootSignature;
        
        void ImmediatelyExecute(std::function<void(ComPtr<ID3D12GraphicsCommandList> cmdList)>&& function);
        
        virtual IGPUBuffer* CreateBuffer(const BufferDesc& desc, void* data) override;
        virtual void UploadBuffer(IGPUBuffer* bufferResource, uint32_t offset, void* data, uint32_t size) override;
        static D3D12_RESOURCE_STATES GetResourceState(BufferResourceState state);
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

### File: `Runtime/Platforms/D3D12/D3D12Struct.h`
```cpp
#include "Graphics/IGPUResource.h"

namespace EngineCore
{


    struct TD3D12ConstantBuffer
    {
        ComPtr<ID3D12Resource> mBufferResource;
        int mSize = 0;
        void* mCpuAddress = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS mGPUAddress = 0;
        DescriptorHandle handleCBV = {};
        int registerSlot = 0;
        TD3D12ConstantBuffer(){};
    };

    struct TD3D12Fence
    {
    public:
        int mCurrentFence;
        Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    };

    struct TD3D12DrawCommand
    {
    public:
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocators;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> commandLists;
    };

    // 存储创建PSO需要的信息
    struct TD3D12PSO
    {
        PSODesc desc;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
        ComPtr<ID3DBlob> vsBlob;
        ComPtr<ID3DBlob> psBlob;
    };
        


} // namespace EngineCore
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

### File: `Runtime/Platforms/D3D12/D3D12Buffer.h`
```cpp
#include "d3dUtil.h"

namespace EngineCore
{
    class D3D12Buffer : public IGPUBuffer
    {
    public:
        D3D12Buffer(ComPtr<ID3D12Resource> resource, const BufferDesc& desc, D3D12_RESOURCE_STATES initialState)
            : m_Resource(resource), m_Desc(desc) 
        {
            if(desc.memoryType == BufferMemoryType::Upload)
            {
                Map();
            }

            switch(initialState)
            {
                case D3D12_RESOURCE_STATE_COMMON:
                m_ResourceState = BufferResourceState::STATE_COMMON;
                break;
                case D3D12_RESOURCE_STATE_GENERIC_READ:
                m_ResourceState = BufferResourceState::STATE_GENERIC_READ;
                break;
                case D3D12_RESOURCE_STATE_COPY_DEST:
                m_ResourceState = BufferResourceState::STATE_COPY_DEST;
                break;
                default:
                    ASSERT("Wrong InitialState");
                break;
            }
        }

        virtual ~D3D12Buffer()
        {
            if(m_Desc.memoryType != BufferMemoryType::Default) UnMap();
        }

        virtual const BufferDesc& GetDesc() const override {return m_Desc;};
        virtual void* GetNativeHandle() const override {return m_Resource.Get();}

    
        virtual uint64_t GetGPUVirtualAddress() const override
        {
            return m_Resource->GetGPUVirtualAddress();
        }
    
        virtual void* Map() override
        {
            if(!m_MappedData){
                m_Resource->Map(0, nullptr, &m_MappedData);
            }
            return m_MappedData;
        }

        virtual void UnMap() override
        {
            if(m_MappedData)
            {
                m_Resource->Unmap(0, nullptr);
                m_MappedData = nullptr;
```
...
```cpp
        }
    
        virtual void SetName(const wchar_t* name) override {m_Resource->SetName(name);}
```

### File: `Runtime/Platforms/D3D12/D3D12DescAllocator.h`
```cpp
#include "Graphics/IGPUResource.h"

namespace EngineCore
{
    class D3D12DescAllocator
    {
    public:
        D3D12DescAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap, bool isShaderVisible = false);
        ~D3D12DescAllocator(){};
        inline D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType(){ return mHeapType;};

        DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
		DescriptorHandle CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
        DescriptorHandle AllocateStaticHandle();
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;

        void Reset();
        void CleanPerFrameData();

        // 设置动态分配的起始位置（用于混合 Heap 模式）
        void SetDynamicStartOffset(int offset) 
        { 
            dynamicStartOffset = offset;
            currDynamicoffset = offset;
        }
        // 专门用于 Global Heap 的动态分配
        DescriptorHandle AllocateDynamicSpace(int count);

    private:
        D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
        D3D12_DESCRIPTOR_HEAP_FLAGS GetHeapVisible(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap);
        int ConfigAllocatorDescSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

        //std::vector<bool> isInUse;
        std::vector<int> freeIndexList;

        int dynamicStartOffset = 0; // 记录动态分配的起始点，Reset 时回到这里
        int currDynamicoffset = 0;
        
        int currentOffset = 0;
        uint32_t startIndex = 0;
        uint32_t mDescriptorSize = 0;
        int maxCount = 0;
    };

}
```

### File: `Runtime/Platforms/D3D12/D3D12DescManager.h`
```cpp
#include "Graphics/IGPUResource.h"

namespace EngineCore
{
    class D3D12DescManager
    {
    public:
        D3D12DescManager();
        ~D3D12DescManager();
        static D3D12DescManager* GetInstance(){return mInstance;};
        static void Create(Microsoft::WRL::ComPtr<ID3D12Device> device);
        static Microsoft::WRL::ComPtr<ID3D12Device> mD3DDevice;
        
        DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
		DescriptorHandle CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, bool isShaderVisible = false);

        inline DescriptorHandle GetFrameCbvSrvUavAllocator(int count) 
        {
            return mBindlessAllocator->AllocateDynamicSpace(count);
        }
        

        void ResetFrameAllocator();
        vector<D3D12DescAllocator> mDescAllocators;
 
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetBindlessCbvSrvUavHeap()
        {
            return mBindlessAllocator->mHeap;
        }


    private:
        static D3D12DescManager* mInstance;
        // Bindless Heap (Shader Visible, Global)
        D3D12DescAllocator* mBindlessAllocator = nullptr;
    };
}
```

### File: `Runtime/Platforms/D3D12/D3D12PSO.h`
```cpp
#include "D3D12Struct.h"

namespace EngineCore
{

    class D3D12PSO
    {
    public:
        static unordered_map<uint32_t, ComPtr<ID3D12PipelineState>> shaderPSOMap;
         
        static ComPtr<ID3D12PipelineState> CreatePSO(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, const TD3D12PSO &pso);
        static ComPtr<ID3D12PipelineState> GetOrCreatePSO(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, PSODesc& psodesc);
        static ComPtr<ID3D12PipelineState> GetOrCreateComputeShaderPSO(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, PSODesc& psodesc);
    private:
        static D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc(const MaterailRenderState& matRenderState);
        static D3D12_BLEND_DESC GetBlendDesc(const MaterailRenderState& matRenderState);
        static D3D12_RASTERIZER_DESC GetRasterizerDesc(const MaterailRenderState& matRenderState);

    };

}
```

### File: `Runtime/Platforms/D3D12/D3D12RootSignature.h`
```cpp
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

    public:
        static ComPtr<ID3D12RootSignature> GetOrCreateARootSig(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, Shader* shader);
        static ComPtr<ID3D12RootSignature> GetOrCreateAComputeShaderRootSig(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, ComputeShader* csShader);
        static ComPtr<ID3D12RootSignature> GetOrCreateARootSig(const RootSignatureKey& key)
        {
            ASSERT(mRootSigMap.count(key) > 0);
            return mRootSigMap[key];
        }
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

### File: `Runtime/Platforms/D3D12/D3D12Texture.h`
```cpp
#include "d3dUtil.h"

namespace EngineCore
{
    // 只是一个资源的壳，IGPUTexture的实现，持有指针
    class D3D12Texture : public IGPUTexture
    {
    public:
        D3D12Texture() = default;

        D3D12Texture(const TextureDesc& desc) 
            : m_Desc(desc)
        {
        }

        D3D12Texture(ComPtr<ID3D12Resource> resource, const TextureDesc& desc, D3D12_RESOURCE_STATES initialState)
            : m_Resource(resource), m_Desc(desc) 
        {
            switch(initialState)
            {
                case D3D12_RESOURCE_STATE_COMMON:
                m_ResourceState = BufferResourceState::STATE_COMMON;
                break;
                case D3D12_RESOURCE_STATE_GENERIC_READ:
                m_ResourceState = BufferResourceState::STATE_GENERIC_READ;
                break;
                case D3D12_RESOURCE_STATE_COPY_DEST:
                m_ResourceState = BufferResourceState::STATE_COPY_DEST;
                break;
                default:
                    ASSERT("Wrong InitialState");
                break;
            }
        }

        virtual ~D3D12Texture()
        {
        }

        virtual const TextureDesc& GetDesc() const override {return m_Desc;};
        virtual void* GetNativeHandle() const override {return m_Resource.Get();}


        virtual uint64_t GetGPUVirtualAddress() const override
        {
            return m_Resource->GetGPUVirtualAddress();
        }

        virtual void SetName(const wchar_t* name) override {m_Resource->SetName(name);}
    public:
        ComPtr<ID3D12Resource> m_Resource;
        TextureDesc m_Desc;
    };
}
```

### File: `Runtime/Platforms/D3D12/d3dx12.h`
```cpp
#if defined( __cplusplus )

struct CD3DX12_DEFAULT {};
```
...
```cpp

//------------------------------------------------------------------------------------------------
struct CD3DX12_RECT : public D3D12_RECT
{
    CD3DX12_RECT()
    {}
    explicit CD3DX12_RECT( const D3D12_RECT& o ) :
        D3D12_RECT( o )
    {}
    explicit CD3DX12_RECT(
        LONG Left,
        LONG Top,
        LONG Right,
        LONG Bottom )
    {
        left = Left;
        top = Top;
        right = Right;
        bottom = Bottom;
    }
    ~CD3DX12_RECT() {}
    operator const D3D12_RECT&() const { return *this; }
};
```
...
```cpp
        back = Back;
    }
    ~CD3DX12_BOX() {}
```
...
```cpp

//------------------------------------------------------------------------------------------------
struct CD3DX12_DEPTH_STENCIL_DESC : public D3D12_DEPTH_STENCIL_DESC
{
    CD3DX12_DEPTH_STENCIL_DESC()
    {}
    explicit CD3DX12_DEPTH_STENCIL_DESC( const D3D12_DEPTH_STENCIL_DESC& o ) :
        D3D12_DEPTH_STENCIL_DESC( o )
    {}
    explicit CD3DX12_DEPTH_STENCIL_DESC( CD3DX12_DEFAULT )
    {
        DepthEnable = TRUE;
        DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        StencilEnable = FALSE;
        StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
        { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
        FrontFace = defaultStencilOp;
        BackFace = defaultStencilOp;
    }
    explicit CD3DX12_DEPTH_STENCIL_DESC(
        BOOL depthEnable,
        D3D12_DEPTH_WRITE_MASK depthWriteMask,
        D3D12_COMPARISON_FUNC depthFunc,
        BOOL stencilEnable,
        UINT8 stencilReadMask,
        UINT8 stencilWriteMask,
        D3D12_STENCIL_OP frontStencilFailOp,
        D3D12_STENCIL_OP frontStencilDepthFailOp,
        D3D12_STENCIL_OP frontStencilPassOp,
        D3D12_COMPARISON_FUNC frontStencilFunc,
        D3D12_STENCIL_OP backStencilFailOp,
        D3D12_STENCIL_OP backStencilDepthFailOp,
        D3D12_STENCIL_OP backStencilPassOp,
        D3D12_COMPARISON_FUNC backStencilFunc )
    {
        DepthEnable = depthEnable;
        DepthWriteMask = depthWriteMask;
        DepthFunc = depthFunc;
        StencilEnable = stencilEnable;
        StencilReadMask = stencilReadMask;
        StencilWriteMask = stencilWriteMask;
        FrontFace.StencilFailOp = frontStencilFailOp;
        FrontFace.StencilDepthFailOp = frontStencilDepthFailOp;
        FrontFace.StencilPassOp = frontStencilPassOp;
        FrontFace.StencilFunc = frontStencilFunc;
        BackFace.StencilFailOp = backStencilFailOp;
        BackFace.StencilDepthFailOp = backStencilDepthFailOp;
        BackFace.StencilPassOp = backStencilPassOp;
        BackFace.StencilFunc = backStencilFunc;
    }
    ~CD3DX12_DEPTH_STENCIL_DESC() {}
    operator const D3D12_DEPTH_STENCIL_DESC&() const { return *this; }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_BLEND_DESC : public D3D12_BLEND_DESC
{
```
...
```cpp
            RenderTarget[ i ] = defaultRenderTargetBlendDesc;
    }
    ~CD3DX12_BLEND_DESC() {}
```