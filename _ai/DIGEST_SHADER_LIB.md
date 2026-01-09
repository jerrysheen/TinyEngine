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
- `[31]` **Assets/Shader/BlitShader.hlsl** *(Content Included)*
- `[31]` **Assets/Shader/GPUCulling.hlsl** *(Content Included)*
- `[8]` **Runtime/Graphics/MaterialLayout.h** *(Content Included)*
- `[8]` **Runtime/Platforms/D3D12/D3D12RootSignature.h** *(Content Included)*
- `[7]` **Runtime/PreCompiledHeader.h** *(Content Included)*
- `[7]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[7]` **Runtime/Graphics/ModelData.h** *(Content Included)*
- `[7]` **Runtime/Graphics/ComputeShader.h** *(Content Included)*
- `[7]` **Runtime/Graphics/ModelUtils.h** *(Content Included)*
- `[7]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[7]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[7]` **Runtime/Graphics/Shader.h** *(Content Included)*
- `[7]` **Runtime/Scene/SceneManager.h** *(Content Included)*
- `[7]` **Runtime/Scene/Scene.h** *(Content Included)*
- `[7]` **Runtime/Resources/ResourceManager.h** *(Content Included)*
- `[7]` **Runtime/Serialization/MetaFactory.h** *(Content Included)*
- `[7]` **Runtime/Serialization/JsonSerializer.h** *(Content Included)*
- `[7]` **Runtime/Serialization/BaseTypeSerialization.h** *(Content Included)*
- `[7]` **Runtime/Serialization/MetaData.h**
- `[7]` **Runtime/Serialization/MetaLoader.h**
- `[7]` **Runtime/Core/PublicStruct.h**
- `[7]` **Runtime/Core/Profiler.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[7]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[7]` **Runtime/Math/AABB.h**
- `[7]` **Runtime/Math/Math.h**
- `[7]` **Runtime/GameObject/Transform.h**
- `[7]` **Runtime/GameObject/GameObject.h**
- `[7]` **Runtime/GameObject/MeshRenderer.h**
- `[7]` **Runtime/GameObject/Camera.h**
- `[7]` **Runtime/GameObject/MeshFilter.h**
- `[7]` **Runtime/Renderer/BatchManager.h**
- `[7]` **Runtime/Renderer/RenderCommand.h**
- `[7]` **Runtime/Renderer/RenderSorter.h**
- `[7]` **Runtime/Renderer/RenderAPI.h**
- `[7]` **Runtime/Renderer/Renderer.h**
- `[7]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[7]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[7]` **Editor/Panel/EditorMainBar.h**
- `[6]` **Runtime/Resources/Resource.h**
- `[6]` **Runtime/Serialization/AssetSerialization.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12PSO.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12DescManager.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h**
- `[6]` **Runtime/Renderer/RenderStruct.h**
- `[6]` **Runtime/Renderer/RenderEngine.h**
- `[6]` **Editor/D3D12/D3D12EditorGUIManager.h**
- `[5]` **premake5.lua**
- `[5]` **Runtime/EngineCore.h**
- `[5]` **Runtime/Graphics/MaterialInstance.h**
- `[5]` **Runtime/Resources/Asset.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12Buffer.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[5]` **Runtime/Math/Matrix4x4.h**
- `[5]` **Runtime/Math/Frustum.h**
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


struct PerObjectData
{
    float4x4 objectToWorld;
    uint matIndex;
    uint renderProxyStartIndex;
    uint renderProxyCount;
    uint padding; 
};
```

### File: `Assets/Shader/SimpleTestShader.hlsl`
```hlsl

// 纹理资源
Texture2D DiffuseTexture : register(t0, space0);
Texture2D NormalTexture : register(t1, space0);
Texture2D SpecularTexture : register(t2, space0);
TextureCube EnvironmentMap : register(t3, space0);

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

            // 此时 offset = 16+16+4+4+8 = 48 bytes
            // 还需要补齐到 16 字节对齐吗？HLSL cbuffer 是 16 字节对齐的
            // 目前 48 刚好是 16 的倍数，完美。
            
            layout.m_TotalSize = currentOffset;
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
    };

}
```

### File: `Runtime/Graphics/ModelData.h`
```cpp
#include "Math/AABB.h"

namespace EngineCore
{
    ModelData* GetFullScreenQuad();
    class ModelData : public Resource
    {
    public:
        // todo: 先这么写，后续或许抽成单独Component
        AABB bounds;
        std::vector<Vertex> vertex;
        std::vector<int> index;
        std::vector<InputLayout> layout;
        ModelData() = default;
        ModelData(MetaData* metaData);
        ModelData(Primitive primitiveType);
    private:
        void ProcessNode(aiNode* node, const aiScene* scene);
        void LoadAiMesh(const string& path);
        void ProcessMesh(aiMesh* aiMesh, const aiScene* scene);
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

### File: `Runtime/Graphics/ModelUtils.h`
```cpp
#include "Graphics/ModelData.h"

namespace EngineCore
{
    class ModelUtils
    {
    public:
        static void GetFullScreenQuad(ModelData* modelData);
    private:

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

### File: `Runtime/Graphics/Material.h`
```cpp
#include "MaterialInstance.h"

namespace EngineCore
{
    class Material : public Resource
    {
    public:
        bool isDirty = true;
        std::unique_ptr<MaterialInstance> matInstance;
        ResourceHandle<Shader> mShader;
        unordered_map<string, IGPUTexture*> textureData;
        unordered_map<std::string, ResourceHandle<Texture>> textureHandleMap;


        Material() = default;
        Material(MetaData* metaData);
        Material(ResourceHandle<Shader> shader);
        Material(const Material& other);
        void UploadDataToGpu();
        ~Material();


        void SetValue(const string& name, void* data, uint32_t size) 
        {
            ASSERT(matInstance != nullptr);
            matInstance->SetValue(name, data, size);
        }

        // 通用设置材质texture的接口
        void SetTexture(const string& name, IGPUTexture* texture)
        {
            ASSERT(textureData.count(name) > 0);
            if(textureHandleMap.count(name))
            {
                textureHandleMap.erase(name);
            }
            textureData[name] = texture;
        }

        // 运行时关联一个临时资源，建立一个引用， 防止资源因为0引用被销毁
        void SetTexture(const string& name, ResourceHandle<Texture> texture)
        {
            ASSERT(textureData.count(name) > 0);
            textureHandleMap[name] = texture;
            if(texture.IsValid())
            {
                textureData[name] = texture.Get()->textureBuffer;
            }
        }

        inline MaterailRenderState GetMaterialRenderState() const { return mRenderState;};
        MaterailRenderState mRenderState;
        BufferAllocation materialAllocation;
    private:
        void LoadDependency(const std::unordered_map<std::string, MetaData>& dependentMap);
        void SetUpRenderState();
        void GetTextureInfoFromShaderReflection();
    };
}
```

### File: `Runtime/Graphics/Shader.h`
```cpp


namespace EngineCore
{
    class Shader : public Resource
    {
    public:
        Shader(MetaData* metaFile);
        ShaderReflectionInfo mShaderReflectionInfo;
        vector<InputLayout> mShaderInputLayout;

        Shader();
        ~Shader();
        string name;
    };
}
```

### File: `Runtime/Scene/SceneManager.h`
```cpp


namespace EngineCore
{
    class Scene;
    class SceneManager
    {
        // 允许Manager类访问SceneManager私有函数。
    public:
        void LoadScene();
        void UnloadScene();
        GameObject* CreateGameObject(const std::string& name);
        GameObject* FindGameObject(const std::string& name);

        void RemoveScene(const std::string& name);
        static void Update();
        static void Create();
        static void Destroy();
        static void EndFrame();
        void Init();
    public:
        inline static SceneManager* GetInstance() 
        {
            if (!s_Instance) 
            {
                s_Instance = new SceneManager();
            }
            return s_Instance;
        }

        SceneManager();
        ~SceneManager();

        // todo： 这部分数据也要找地方存， maybe一个Global的渲染处

        Material* blitMaterial;
        ModelData* quadMesh;
        ResourceHandle<Shader> blitShader;
        ResourceHandle<Texture> testTexture;

        inline Scene* GetCurrentScene() 
        { 
            return mCurrentScene; 
        };
        inline void SetCurrentScene(Scene* scene) 
        { 
            mCurrentScene = scene; 
        };
        Scene* AddNewScene(const std::string& name);
        void SwitchSceneTo(const std::string& name);

    private:
        static SceneManager* s_Instance;
        Scene* mCurrentScene = nullptr;
        unordered_map<std::string, Scene*> mSceneMap;
        vector<ResourceHandle<Texture>> texHandler;
    };

}
```

### File: `Runtime/Scene/Scene.h`
```cpp
#include "Renderer/RenderStruct.h"

namespace EngineCore
{
    struct RenderSceneData
    {
        vector<MeshRenderer*> meshRendererList;
        vector<uint32_t> vaoIDList;
        vector<AABB> aabbList;
        vector<Matrix4x4> objectToWorldMatrixList;
        vector<uint32_t> layerList;

        // 每帧重置
        vector<uint32_t> transformDirtyList;
        vector<uint32_t> materialDirtyList;
        struct RenderSceneData() = default;
        inline void SyncData(MeshRenderer* renderer, uint32_t index)
        {
            meshRendererList[index] = renderer;
            if (renderer && renderer->gameObject)
            {
                aabbList[index] = renderer->worldBounds;
                objectToWorldMatrixList[index] = renderer->gameObject->transform->GetWorldMatrix();
                auto* meshFilter = renderer->gameObject->GetComponent<MeshFilter>();
                if(meshFilter != nullptr)
                {
                    vaoIDList[index] = meshFilter->mMeshHandle.Get()->GetInstanceID();
                }
                else
                {
                    vaoIDList[index] = UINT32_MAX;
                }
            }
        }

        inline void PushNewData() 
        {
            meshRendererList.emplace_back();
            vaoIDList.emplace_back();
            aabbList.emplace_back();
            objectToWorldMatrixList.emplace_back();
            layerList.emplace_back();
        }


        inline void DeleteData(uint32_t index)
        {
            meshRendererList[index] = nullptr;
            vaoIDList[index] = UINT32_MAX;

            // 后续删除RenderProxy
        }

        inline void ClearDirtyList()
        {
            materialDirtyList.clear();
            transformDirtyList.clear();
        }

        inline void UpdateDirtyRenderNode()
```
...
```cpp
    };

    class Scene : Object
    {
    public:
        Scene();
        ~Scene();
        Scene(const std::string& name):name(name){};
        void Open();
        void Close(){};
        void Update();
        void EndFrame();
        GameObject* FindGameObject(const std::string& name);
        GameObject* CreateGameObject(const std::string& name);
        void Scene::DestroyGameObject(const std::string& name);

        void AddCamToStack(Camera* cam);
        inline void SetMainCamera(Camera* cam) { mainCamera = cam; }
        inline Camera* GetMainCamera() { return mainCamera; }
        
        // 只在GameObject层用到，析构和SetParent的时候， 创建的时候调用
        void RemoveGameObjectToSceneList(GameObject* object);
        void AddGameObjectToSceneList(GameObject* object);

        void AddRootGameObject(GameObject* object);
        void TryRemoveRootGameObject(GameObject* object);
        
        //todo: 先用vector写死，后面要用priorityqueue之类的
        std::vector<Camera*> cameraStack;

        void RunLogicUpdate();
        void RunTransformUpdate();
        void RunRecordDirtyRenderNode();


        // todo:
        // 材质更新的时候， 也需要去调用这个逻辑，比如changeRenderNodeInfo之类的，
        int AddNewRenderNodeToCurrentScene(MeshRenderer* renderer);
        void DeleteRenderNodeFromCurrentScene(uint32_t index);
        
    public:
        int m_CurrentSceneRenderNodeIndex = 0;
        std::queue<uint32_t> m_FreeSceneNode;
        std::string name;
        std::vector<GameObject*> allObjList;
        std::vector<GameObject*> rootObjList;
        bool enabled = true;
        Camera* mainCamera = nullptr;
        RenderSceneData renderSceneData;
    private:
    };    
```

### File: `Runtime/Resources/ResourceManager.h`
```cpp


namespace EngineCore
{
    class ResourceManager
    {
    public:
        
        static void Create();
        static void Delete();
        ~ResourceManager(){};
        ResourceManager(){};

        // todo :资源正确释放
        static void Destroy()
        {
            delete sInstance;
            sInstance = nullptr;
        };


        template<typename T>
        ResourceHandle<T> LoadAsset(const string& relativePath)
        {
            string path = PathSettings::ResolveAssetPath(relativePath);
            // 根据path判断是否加载， 加载了就存ID和path:
            if(mPathToID.count(path) > 0)
            {
                AssetID& id = mPathToID[path];
                return ResourceHandle<T>(id);
            }

            // 加载meta文件，指向一个具体的MetaData文件
            // todo： 循环检测
            // todo: 还没做依赖， prefab的时候做
            MetaData* metaData = MetaLoader::LoadMetaData<T>(path);
            
            // 没必要在这个地方做dependency的加载，而是在具体加载的时候，这样子的话可以知道
            // 具体的dependency类型
            
            // 创建资源的时候生成对应的AssetID
            Resource* resource = new T(metaData);
            ASSERT(resource != nullptr);
            
            AssetID id = resource->GetAssetID();
            mPathToID[path] = id;
            mResourceCache[id] = resource;
            delete metaData;
            return ResourceHandle<T>(id);
        }

        template<typename T>
        ResourceHandle<T> Instantiate(const ResourceHandle<T>& sourceHandle)
        {
            T* src = sourceHandle.Get();
            ASSERT(src != nullptr);

            Resource* resource = new T(*src);
            ASSERT(resource != nullptr);
            // 将metaData设置填入Resource中
```
...
```cpp
        static inline ResourceManager* GetInstance()
        {
            ASSERT(sInstance != nullptr);
            return sInstance;
        };

        template<typename T>
        inline T* GetResource(AssetID id) 
        {
```
...
```cpp
            // 创建资源的时候无复用关系 应该是直接创建
            Resource* resource = new T(std::forward<Args>(args)...);
            ASSERT(resource != nullptr);
            // 将metaData设置填入Resource中
            AssetID id = resource->GetAssetID();
            mResourceCache[id] = resource;
            return ResourceHandle<T>(id);
        }

    // todo: temp public for Editor Test
    public:
        void LoadDependencies(const std::vector<MetaData>& dependenciesList);
        static ResourceManager* sInstance;
        unordered_map<AssetID, Resource*> mResourceCache;
        unordered_map<string, AssetID> mPathToID;
        std::vector<Resource*> mPendingDeleteList;
        void RegisterMaterial(const Material* mat);
        void UnRegisterMaterial(const Material* mat);
        // todo: 之后会把所有资源类，放在一个线性分配器里面
        //std::vector<Material*> mAllMaterialData;
    };

}
```

### File: `Runtime/Serialization/MetaFactory.h`
```cpp

// 提供各种反序列化操作工厂，比如输入一个json，输出一个反序列化完的GameObject
namespace EngineCore
{
    using json = nlohmann::json;

    class MetaFactory
    {
    public:
        using json = nlohmann::json;

        static GameObject* CreateGameObjectFromMeta(const json& json);
        static Scene* CreateSceneFromMeta(const json& json);

        // 非特化的转换
        template<typename T>
        inline json static ConvertToJson(const T* data)
        {
            return *data;
        }

        template<>
        inline json static ConvertToJson<GameObject>(const GameObject* obj)
        {

            json componentsArray = json::array();

            // 遍历所有组件，手动根据类型调用对应的序列化
            for (const auto& [type, comp] : obj->components)
            {
                json compJson;
                compJson["Type"] = comp->GetScriptName();
                compJson["Data"] = comp->SerializedFields();
                componentsArray.push_back(compJson);
            }
    
            // 递归序列化子对象
            json childrenArray = json::array();
            for (GameObject* child : obj->GetChildren()) {  // 或 obj->GetChildren()
                childrenArray.push_back(ConvertToJson<GameObject>(child));  
            }
    
            json j = json{ 
                {"Enabled", obj->enabled},
                {"Name", obj->name},
                {"Component", componentsArray},
                {"ChildGameObject", childrenArray}
            };

            return j;
        }


        template<>
        inline json static ConvertToJson<Scene>(const Scene* scene)
        {
            json rootObjectArray = json::array();

            // 遍历所有组件，手动根据类型调用对应的序列化
            for (const auto& go : scene->rootObjList)
```

### File: `Runtime/Serialization/JsonSerializer.h`
```cpp

// 这个类负责读取和写回json，不负责别的功能。
namespace EngineCore
{
    using json = nlohmann::json;

    class JsonSerializer
    {
    public:
        template<typename T>
        static void SvaeAsJson(const T* data, const string& relativePath)
        {
            string metaPath = PathSettings::ResolveAssetPath(relativePath);

            // 防御：如果路径中有扩展名，去掉扩展名
            size_t dotPos = metaPath.find_last_of('.');
            if (dotPos != string::npos) {
                metaPath = metaPath.substr(0, dotPos);
            }

            metaPath += ".meta";
            
            json j = MetaFactory::ConvertToJson<T>(data);

            // 确保目录存在
            std::filesystem::path filePath(metaPath);
            std::filesystem::path dirPath = filePath.parent_path();
            if (!dirPath.empty() && !std::filesystem::exists(dirPath)) {
                std::filesystem::create_directories(dirPath);
            }

            // 写入文件（不存在创建，存在覆盖）
            std::ofstream file(metaPath, std::ios::out | std::ios::trunc);
            if (!file.is_open()) {  
                ASSERT_MSG(false, "Failed to open/create file!");
                return;
            }
            
            file << j.dump(4);
            file.close();
            
            return;
        };

    
        static json ReadFromJson(const string& path)
        {
			string metaPath = EngineCore::PathSettings::ResolveAssetPath(path);
    
			// 读取JSON文件并解析
			std::ifstream file(metaPath);

			if(!file.is_open())
			{
				// 错误处理：文件打不开
				std::cerr << "无法打开文件: " << metaPath << std::endl;
                return json{};
			}
			
			json j = json::parse(file);
```

### File: `Runtime/Serialization/BaseTypeSerialization.h`
```cpp
#include "Graphics/IGPUResource.h"

namespace EngineCore
{
    // // Vector3
    using json = nlohmann::json;
    inline void to_json(json& j, const Vector3& v)
    {
        j = json{ {"x", v.x}, {"y", v.y}, {"z", v.z} };
    }

    inline void from_json(const json& j, EngineCore::Vector3& v)
    {
        j.at("x").get_to(v.x);
        j.at("y").get_to(v.y);
        j.at("z").get_to(v.z);
    }

    //vector2
    inline void to_json(json& j, const EngineCore::Vector2& v)
    {
        j = json{ {"x", v.x}, {"y", v.y}};
    }

    inline void from_json(const json& j, EngineCore::Vector2& v)
    {
        j.at("x").get_to(v.x);
        j.at("y").get_to(v.y);
    }

    // Matrix4x4
    inline void to_json(json& j, const EngineCore::Matrix4x4& m)
    {
        j = json{
            {"m00", m.m00}, {"m01", m.m01}, {"m02", m.m02},{"m03", m.m03},
            {"m10", m.m10}, {"m11", m.m11}, {"m12", m.m12}, {"m13", m.m13},
            {"m20", m.m20}, {"m21", m.m21}, {"m22", m.m22}, {"m23", m.m23},
            {"m30", m.m30}, {"m31", m.m31}, {"m32", m.m32}, {"m33", m.m33}
        };
    }

    inline void from_json(const json& j, EngineCore::Matrix4x4& m)
    {
        j.at("m00").get_to(m.m00); j.at("m01").get_to(m.m01); j.at("m02").get_to(m.m02); j.at("m03").get_to(m.m03);
        j.at("m10").get_to(m.m10); j.at("m11").get_to(m.m11); j.at("m12").get_to(m.m12); j.at("m13").get_to(m.m13);
        j.at("m20").get_to(m.m20); j.at("m21").get_to(m.m21); j.at("m22").get_to(m.m22); j.at("m23").get_to(m.m23);
        j.at("m30").get_to(m.m30); j.at("m31").get_to(m.m31); j.at("m32").get_to(m.m32); j.at("m33").get_to(m.m33);
    }

    // Quaternion
    inline void to_json(json& j, const EngineCore::Quaternion& m)
    {
        j = json{
            {"x", m.x},
            {"y", m.y},
            {"z", m.z},
            {"w", m.w},
        };
    }

```