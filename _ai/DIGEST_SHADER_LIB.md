# Architecture Digest: SHADER_LIB
> Auto-generated. Focus: Assets/Shader, hlsl, glsl, include, Lighting, Surface

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- Shader库是GPU驱动渲染的核心资产。
- 关注通用Shader结构、CBuffer布局与Pass一致性。

## Key Files Index
- `[42]` **Assets/Shader/include/Core.hlsl** *(Content Included)*
- `[34]` **Assets/Shader/SimpleTestShader.hlsl** *(Content Included)*
- `[34]` **Assets/Shader/StandardPBR.hlsl** *(Content Included)*
- `[34]` **Assets/Shader/StandardPBR_VertexPulling.hlsl** *(Content Included)*
- `[31]` **Assets/Shader/BlitShader.hlsl** *(Content Included)*
- `[31]` **Assets/Shader/GPUCulling.hlsl** *(Content Included)*
- `[8]` **Runtime/Graphics/MaterialLayout.h** *(Content Included)*
- `[8]` **Runtime/Serialization/DDSTextureLoader.h** *(Content Included)*
- `[8]` **Runtime/Platforms/D3D12/D3D12RootSignature.h** *(Content Included)*
- `[7]` **Runtime/PreCompiledHeader.h** *(Content Included)*
- `[7]` **Runtime/Core/Profiler.h** *(Content Included)*
- `[7]` **Runtime/Core/PublicStruct.h** *(Content Included)*
- `[7]` **Runtime/GameObject/Camera.h** *(Content Included)*
- `[7]` **Runtime/GameObject/GameObject.h** *(Content Included)*
- `[7]` **Runtime/GameObject/MeshFilter.h** *(Content Included)*
- `[7]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[7]` **Runtime/Graphics/ComputeShader.h** *(Content Included)*
- `[7]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[7]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[7]` **Runtime/Graphics/Material.h**
- `[7]` **Runtime/Graphics/Mesh.h**
- `[7]` **Runtime/Math/AABB.h**
- `[7]` **Runtime/Math/Math.h**
- `[7]` **Runtime/Renderer/BatchManager.h**
- `[7]` **Runtime/Renderer/RenderAPI.h**
- `[7]` **Runtime/Renderer/RenderCommand.h**
- `[7]` **Runtime/Renderer/Renderer.h**
- `[7]` **Runtime/Renderer/RenderSorter.h**
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
- `[7]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[7]` **Editor/Panel/EditorMainBar.h**
- `[6]` **Runtime/GameObject/Transform.h**
- `[6]` **Runtime/Graphics/MaterialData.h**
- `[6]` **Runtime/Graphics/MeshUtils.h**
- `[6]` **Runtime/Graphics/Shader.h**
- `[6]` **Runtime/Renderer/RenderEngine.h**
- `[6]` **Runtime/Renderer/RenderStruct.h**
- `[6]` **Runtime/Serialization/StreamHelper.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12DescManager.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12PSO.h**
- `[6]` **Editor/D3D12/D3D12EditorGUIManager.h**
- `[5]` **premake5.lua**
- `[5]` **Runtime/EngineCore.h**
- `[5]` **Runtime/Core/ThreadSafeQueue.h**
- `[5]` **Runtime/Graphics/MaterialInstance.h**
- `[5]` **Runtime/Graphics/Texture.h**
- `[5]` **Runtime/Math/Frustum.h**
- `[5]` **Runtime/Math/Matrix4x4.h**
- `[5]` **Runtime/Renderer/Culling.h**
- `[5]` **Runtime/Renderer/RenderContext.h**

## Evidence & Implementation Details

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

### File: `Runtime/Graphics/MaterialLayout.h`
```cpp
#include "Assert.h"

namespace EngineCore
{
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
        // 后续用json加载
        void BuildFromJson(){};

        // temp;
        // 【新增】硬编码构建一个标准布局，模拟未来的 JSON 加载结果
        // 对应 Shader/SimpleTestShader.hlsl 里的 cbuffer 结构
        static MaterialLayout GetDefaultPBRLayout()
        {
            MaterialLayout layout;
            uint32_t currentOffset = 0;

            // 辅助 Lambda，模拟 JSON 遍历过程
            auto AddProp = [&](const std::string& name, ShaderVariableType type, uint32_t size) 
            {
                MaterialPropertyLayout prop;
                prop.name = name;
                prop.type = type;
                prop.size = size;
                prop.offset = currentOffset;
                layout.m_PropertyLayout[name] = prop;
                
                currentOffset += size;
            };

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
```
...
```cpp
        uint32_t GetPropertyOffset(const std::string& name)
        {
            ASSERT(m_PropertyLayout.count(name) > 0);
            return m_PropertyLayout[name].offset;
        };

        uint32_t GetSize(){ return m_TotalSize;}
```

### File: `Runtime/Serialization/DDSTextureLoader.h`
```cpp
  HEADER      124
  HEADER_DX10* 20	(https://msdn.microsoft.com/en-us/library/bb943983(v=vs.85).aspx)
  PIXELS      fseek(f, 0, SEEK_END); (ftell(f) - 128) - (fourCC == "DX10" ? 17 or 20 : 0)
* the link tells you that this section isn't written unless its a DX10 file
Supports DXT1, DXT3, DXT5.
The problem with supporting DX10 is you need to know what it is used for and how opengl would use it.
File Byte Order:
typedef unsigned int DWORD;           // 32bits little endian
  type   index    attribute           // description
///////////////////////////////////////////////////////////////////////////////////////////////
  DWORD  0        file_code;          //. always `DDS `, or 0x20534444
  DWORD  4        size;               //. size of the header, always 124 (includes PIXELFORMAT)
  DWORD  8        flags;              //. bitflags that tells you if data is present in the file
                                      //      CAPS         0x1
                                      //      HEIGHT       0x2
                                      //      WIDTH        0x4
                                      //      PITCH        0x8
                                      //      PIXELFORMAT  0x1000
                                      //      MIPMAPCOUNT  0x20000
                                      //      LINEARSIZE   0x80000
                                      //      DEPTH        0x800000
  DWORD  12       height;             //. height of the base image (biggest mipmap)
  DWORD  16       width;              //. width of the base image (biggest mipmap)
  DWORD  20       pitchOrLinearSize;  //. bytes per scan line in an uncompressed texture, or bytes in the top level texture for a compressed texture
                                      //     D3DX11.lib and other similar libraries unreliably or inconsistently provide the pitch, convert with
                                      //     DX* && BC*: max( 1, ((width+3)/4) ) * block-size
                                      //     *8*8_*8*8 && UYVY && YUY2: ((width+1) >> 1) * 4
                                      //     (width * bits-per-pixel + 7)/8 (divide by 8 for byte alignment, whatever that means)
  DWORD  24       depth;              //. Depth of a volume texture (in pixels), garbage if no volume data
  DWORD  28       mipMapCount;        //. number of mipmaps, garbage if no pixel data
  DWORD  32       reserved1[11];      //. unused
  DWORD  76       Size;               //. size of the following 32 bytes (PIXELFORMAT)
  DWORD  80       Flags;              //. bitflags that tells you if data is present in the file for following 28 bytes
                                      //      ALPHAPIXELS  0x1
                                      //      ALPHA        0x2
                                      //      FOURCC       0x4
                                      //      RGB          0x40
                                      //      YUV          0x200
                                      //      LUMINANCE    0x20000
  DWORD  84       FourCC;             //. File format: DXT1, DXT2, DXT3, DXT4, DXT5, DX10. 
  DWORD  88       RGBBitCount;        //. Bits per pixel
  DWORD  92       RBitMask;           //. Bit mask for R channel
  DWORD  96       GBitMask;           //. Bit mask for G channel
  DWORD  100      BBitMask;           //. Bit mask for B channel
  DWORD  104      ABitMask;           //. Bit mask for A channel
  DWORD  108      caps;               //. 0x1000 for a texture w/o mipmaps
                                      //      0x401008 for a texture w/ mipmaps
                                      //      0x1008 for a cube map
  DWORD  112      caps2;              //. bitflags that tells you if data is present in the file
                                      //      CUBEMAP           0x200     Required for a cube map.
                                      //      CUBEMAP_POSITIVEX 0x400     Required when these surfaces are stored in a cube map.
                                      //      CUBEMAP_NEGATIVEX 0x800     ^
                                      //      CUBEMAP_POSITIVEY 0x1000    ^
                                      //      CUBEMAP_NEGATIVEY 0x2000    ^
                                      //      CUBEMAP_POSITIVEZ 0x4000    ^
                                      //      CUBEMAP_NEGATIVEZ 0x8000    ^
                                      //      VOLUME            0x200000  Required for a volume texture.
  DWORD  114      caps3;              //. unused
  DWORD  116      caps4;              //. unused
  DWORD  120      reserved2;          //. unused
```
...
```cpp

namespace EngineCore{
    struct DDSHeader {
        uint32_t magic;              // 'DDS ' (0x20534444)
        uint32_t fileSize;               // 124
        uint32_t flags;
        uint32_t height;
        uint32_t width;
        uint32_t pitchOrLinearSize;
        uint32_t depth;
        uint32_t mipMapCount;
        uint32_t reserved1[11];
        uint32_t size;           // 应该是32
        uint32_t flagsData;
        uint32_t fourCC;
        uint32_t rgbBitCount;
        uint32_t rBitMask;
        uint32_t gBitMask;
        uint32_t bBitMask;
        uint32_t aBitMask;
        uint32_t caps;
        uint32_t caps2;
        uint32_t caps3;
        uint32_t caps4;
        uint32_t reserved2;
    };
```
...
```cpp
    };

    struct DDSLoadResult {
        bool success = false;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t mipMapCount = 0;
        TextureFormat format;        // DXT1/DXT3/DXT5标识
        uint32_t blockSize = 0;     // 8 for DXT1, 16 for DXT3/DXT5
        std::vector<uint8_t> pixelData;
    };
```
...
```cpp
                uint32_t mipSize = CalculateDXTMipSize(mipWidth, mipHeight, ddsResult.blockSize);
                
                if (i > 0) {
                    tex->textureDesc.mipOffset[i] = offset;  // 记录当前 mip 的 offset
                }
```
...
```cpp
        }
    
        std::vector<uint8_t> LoadMipData(const std::string& realativePath, int mipCount){}
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

### File: `Runtime/Core/Profiler.h`
```cpp

#if ENABLE_PROFILER
namespace EngineCore
{
    // 函数打点
    struct ProfilerEvent
    {
        const char* name        = nullptr;
        uint32_t    threadID    = 0;
        float       startMs     = 0.0f;
        float       endMs       = 0.0f;
        uint8_t     depth       = 0;
    };

    // 统计数据打点
    struct ProfilerCounter
    {
        const char* name    = nullptr;
        double      value   = 0.0;
    };
    
    struct ProfilerFrame
    {
        float frameStartMs = 0;
        const char* frameTag = nullptr;

        std::vector<ProfilerEvent> events;

        std::unordered_map<std::string, uint32_t> counterIndex;

        void Clear()
        {
            events.clear();
            counterIndex.clear();
            frameTag = nullptr;
            frameStartMs = 0.0f;
        }
    };


    class Profiler
    {
    public:
        static Profiler& Get()
        {
            static Profiler s_Instance;
            return s_Instance;
        }

        void MarkFrame(const char* name)
        {
            const uint32_t nextIndex = (m_currentFrameIndex + 1u) % kMaxFrames;
            m_currentFrameIndex = nextIndex;

            ProfilerFrame& frame = m_frams[nextIndex];
            std::lock_guard<std::mutex> lock(m_frameMutex);
            frame.Clear();
            frame.frameStartMs = GetTimeMs();
            frame.frameTag = name;    
        }
```
...
```cpp
        {
            ProfilerFrame& frame = m_frams[m_currentFrameIndex];
            std::lock_guard<std::mutex> lock(m_frameMutex);
            frame.events.push_back(ProfilerEvent
                {
                    name, threadId, startMs, endMs, depth
                }
```
...
```cpp
        {
            ProfilerFrame& frame = m_frams[m_currentFrameIndex];
            std::lock_guard<std::mutex> lock(m_frameMutex);

            if(frame.counterIndex.count(name) > 0)
            {
                frame.counterIndex[name] += value;
            }
```
...
```cpp
            const uint32_t prevIndex = (m_currentFrameIndex > 0)
                ? (m_currentFrameIndex - 1)
                : (kMaxFrames - 1);
            return m_frams[prevIndex];  // 使用传入的 index
        }

        const ProfilerFrame& GetFrame(uint32_t index) const
        {
```
...
```cpp
        }

        uint32_t GetCurrentFrameIndex() const {return m_currentFrameIndex;}
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


```
...
```cpp
    class Transform;
    class MeshRenderer;
    class MeshFilter;

    struct RenderPacket
    {
        // 为了测试，先用直接塞数据的方式。
        uint64_t sortingKey = 0;
        MeshRenderer* meshRenderer;
        MeshFilter* meshFilter;
        float distanToCamera = 0;
    };
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

### File: `Runtime/GameObject/GameObject.h`
```cpp
#include "MonoBehaviour.h"

namespace EngineCore
{
    class Component;
    class Scene;
    class Transform;
    //class Transform;
    // later we will inherete from Object, now just keep it simple.
    class GameObject : Object
    {
    public:
        GameObject();

        GameObject::GameObject(const std::string& name, Scene* scene);

        ~GameObject();
        void SetParent(const GameObject* gameObject);
        template<typename T>
        inline T* GetComponent() const;
        template<typename T>
        inline T* AddComponent();

        std::vector<GameObject*> GetChildren() const;
    public:
        Transform* transform;
        std::unordered_map<ComponentType, Component*> components;
        std::vector<MonoBehaviour*> scripts;
        std::string name;
        bool enabled = true;
            // 非模板方式
        void AddComponent(Component* compont);
    private:
        Scene* ownerScene = nullptr;
    };

    template<typename T>
    inline T* GameObject::GetComponent() const
    {
        // const 后不能直接用 conponents[type]， 因为可能会产生修改。
        ComponentType type = T::GetStaticType();
        auto it = components.find(type);
        if(it != components.end())
        {
            return static_cast<T*>(it->second);            
        }
        return nullptr;
    }

    template<typename T>
    inline T* GameObject::AddComponent()
    {
        // todo: 这边确实该用multimap的， 因为原则上MonoBehaviour可以挂多个

        ComponentType type = T::GetStaticType();
        if(components.count(type) > 0)
        {
            return nullptr;
        }
        T* component = new T(this);
```

### File: `Runtime/GameObject/MeshFilter.h`
```cpp
#include "Math/AABB.h"

namespace EngineCore
{
    class MeshFilter : public Component
    {
        class GameObejct;
    public:
        MeshFilter() = default;
        MeshFilter(GameObject* gamObject);

        virtual ~MeshFilter() override;
        static ComponentType GetStaticType() { return ComponentType::MeshFilter; };
        virtual ComponentType GetType() const override{ return ComponentType::MeshFilter; };
        void OnLoadResourceFinished();
    public:
        ResourceHandle<Mesh> mMeshHandle;
        
        virtual const char* GetScriptName() const override { return "MeshFilter"; }

        uint32_t GetHash()
        {
            return mMeshHandle->GetInstanceID();
        }
    private:
        uint32_t hash;
    };

}
```

### File: `Runtime/GameObject/MeshRenderer.h`
```cpp


namespace EngineCore
{
    class MeshRenderer : public Component
    {
        class GameObejct;
    public:
        MeshRenderer() = default;
        MeshRenderer(GameObject* gamObject);
        virtual ~MeshRenderer() override;
        static ComponentType GetStaticType() { return ComponentType::MeshRenderer; };
        virtual ComponentType GetType() const override{ return ComponentType::MeshRenderer; };

        virtual const char* GetScriptName() const override { return "MeshRenderer"; }
        
        void SetUpMaterialPropertyBlock();

        inline Material* GetSharedMaterial()
        { 
            return mShardMatHandler.IsValid() ? mShardMatHandler.Get() : nullptr;
        };

        inline void SetSharedMaterial(const ResourceHandle<Material>& mat) 
        {
            mShardMatHandler = mat;
            SetUpMaterialPropertyBlock();
        }

        // return a new Material Instance;
        Material* GetOrCreateMatInstance();
        // 
        ResourceHandle<Material> GetMaterial();
        inline bool HasMaterialOverride() { return mInstanceMatHandler.IsValid(); }

        void UpdateBounds(const AABB& localBounds, const Matrix4x4& worldMatrix);
        uint32_t lastSyncTransformVersion = 0;
        bool shouldUpdateMeshRenderer = true;

        AABB worldBounds;
        uint32_t sceneRenderNodeIndex = UINT32_MAX;
        bool materialDirty = true;
		
        void TryAddtoBatchManager();

        uint32_t renderLayer = 1;
    private:
        ResourceHandle<Material> mShardMatHandler;
        ResourceHandle<Material> mInstanceMatHandler;

    };

}
```

### File: `Runtime/Graphics/ComputeShader.h`
```cpp
#include "Graphics/IGPUResource.h"

namespace EngineCore
{
    class ComputeShader : public Resource
    {
    public:
        IGPUBuffer* GetBufferResource(const std::string& slotName)
        {
            ASSERT(resourceMap.count(slotName) > 0);
            return resourceMap[slotName];
        }

        ComputeShader(const string& path);
        ShaderReflectionInfo mShaderReflectionInfo;
        std::unordered_map<std::string, IGPUBuffer*> resourceMap;
        void SetBuffer(const std::string& name, IGPUBuffer* buffer);
    };
}
```

### File: `Runtime/Graphics/GPUBufferAllocator.h`
```cpp
#include <mutex>

namespace EngineCore
{
    // A unified GPU buffer allocator.
    // Allocates small chunks of memory from a large GPU buffer.
    // Handles free list management for reusable blocks.
    class GPUBufferAllocator : public IGPUBufferAllocator
    {
    public:
        GPUBufferAllocator(const BufferDesc &usage);
        virtual ~GPUBufferAllocator();
        
        void Destory();

        // Allocates a block of 'size' bytes.
        // The allocator will search for a suitable free block or append to the end.
        virtual BufferAllocation Allocate(uint32_t size) override;
        
        // Frees an allocation, making its range available for reuse.
        virtual void Free(const BufferAllocation& allocation) override;
        
        // Resets the allocator, clearing all allocations (effectively freeing everything).
        // Useful for per-frame allocators.
        virtual void Reset() override;
        
        virtual uint64_t GetBaseGPUAddress() const override;
        virtual void UploadBuffer(const BufferAllocation& alloc, void* data, uint32_t size) override;
        virtual IGPUBuffer* GetGPUBuffer() override;
        BufferDesc bufferDesc;
    private:

        IGPUBuffer* m_Buffer = nullptr;
        uint64_t m_MaxSize = 0;
        uint64_t m_CurrOffset = 0; // Tracks the end of the used contiguous space

        // Keeps track of free ranges [offset, size]
        // This is a simple implementation; for high fragmentation scenarios, 
        // a more complex structure (like a segregated free list or RB tree) might be needed.
        struct FreeRange
        {
            uint64_t offset;
            uint32_t size;
        };
        std::vector<FreeRange> m_FreeRanges;
        
        // Helper to find a free block
        bool FindFreeBlock(uint32_t size, uint64_t& outOffset);
    };
}
```

### File: `Runtime/Graphics/GPUSceneManager.h`
```cpp
#include "Resources/ResourceHandle.h"

namespace EngineCore
{

    class GPUSceneManager
    {
    public:
        static GPUSceneManager* GetInstance();
        GPUSceneManager();
        static void Create();
        void Tick();
        void Destroy();
        
        BufferAllocation GetSinglePerMaterialData();
        void RemoveSinglePerMaterialData(const BufferAllocation& bufferalloc);
        void UpdateSinglePerMaterialData(const BufferAllocation& bufferalloc, void* data);

        void TryFreeRenderProxyBlock(uint32_t index);
        void TryCreateRenderProxyBlock(uint32_t index);
        BufferAllocation LagacyRenderPathUploadBatch(void *data, uint32_t size);
        void FlushBatchUploads();
        void UpdateRenderProxyBuffer(const vector<uint32_t>& materialDirtyList);
        void UpdateAABBandPerObjectBuffer(const vector<uint32_t>& transformDirtyList, const vector<uint32_t>& materialDirtyList);

        vector<PerObjectData> perObjectDataBuffer;

        LinearAllocator* perFramelinearMemoryAllocator;

        GPUBufferAllocator* allMaterialDataBuffer;
        GPUBufferAllocator* allObjectDataBuffer;
        GPUBufferAllocator* perFrameBatchBuffer;
        GPUBufferAllocator* allAABBBuffer;
        GPUBufferAllocator* renderProxyBuffer;


        BufferAllocation visiblityAlloc;
        GPUBufferAllocator* visibilityBuffer;
        
        ResourceHandle<ComputeShader> GPUCullingShaderHandler;
    private:
        static GPUSceneManager* sInstance; 
        vector<CopyOp> mPendingBatchCopies;
    };

}
```