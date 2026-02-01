# Architecture Digest: MATERIAL_SYSTEM
> Auto-generated. Focus: Runtime/MaterialLibrary, BistroSceneLoader, Material, MaterialInstance, MaterialLayout, MaterialArchetypeRegistry, MaterialArchetype, MaterialArchytype, StandardPBR, BindlessTexture, MaterialRawByteBuffer

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 材质序列化与反序列化，运行时设置的系统，生成一个MaterialInstance，对应GPU上一小块内存，已经做了MaterialRawByteBuffer，和BindlessTexture
- 关注序列化反序列化是否合理

## Key Files Index
- `[87]` **Runtime/MaterialLibrary/MaterialArchetypeRegistry.h** *(Content Included)*
- `[64]` **Runtime/MaterialLibrary/MaterialInstance.h** *(Content Included)*
- `[57]` **Runtime/MaterialLibrary/MaterialLayout.h** *(Content Included)*
- `[54]` **Runtime/MaterialLibrary/StandardPBR.h** *(Content Included)*
- `[51]` **Runtime/MaterialLibrary/StandardPBR.cpp** *(Content Included)*
- `[41]` **Runtime/Serialization/MaterialLoader.h** *(Content Included)*
- `[37]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[33]` **Runtime/Graphics/Material.cpp** *(Content Included)*
- `[29]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[28]` **Runtime/Scene/BistroSceneLoader.h** *(Content Included)*
- `[26]` **Assets/Shader/StandardPBR.hlsl** *(Content Included)*
- `[26]` **Assets/Shader/StandardPBR_VertexPulling.hlsl** *(Content Included)*
- `[20]` **Runtime/Entry.cpp** *(Content Included)*
- `[13]` **Runtime/Graphics/GPUSceneManager.cpp** *(Content Included)*
- `[12]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[12]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[12]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[11]` **Runtime/Renderer/Renderer.h** *(Content Included)*
- `[7]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[7]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[7]` **Runtime/Renderer/RenderAPI.h**
- `[7]` **Runtime/Renderer/RenderCommand.h**
- `[7]` **Runtime/Renderer/RenderStruct.h**
- `[7]` **Runtime/Resources/ResourceManager.cpp**
- `[7]` **Runtime/Serialization/SceneLoader.h**
- `[7]` **Editor/Panel/EditorMainBar.h**
- `[6]` **Runtime/Renderer/BatchManager.h**
- `[6]` **Runtime/Scene/Scene.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[6]` **Assets/Shader/BlitShader.hlsl**
- `[6]` **Assets/Shader/SimpleTestShader.hlsl**
- `[5]` **Runtime/Core/PublicStruct.h**
- `[5]` **Runtime/GameObject/MeshRenderer.cpp**
- `[5]` **Runtime/Renderer/Renderer.cpp**
- `[5]` **Runtime/Resources/AssetTypeTraits.h**
- `[5]` **Runtime/Resources/ResourceManager.h**
- `[5]` **Runtime/Scene/Scene.cpp**
- `[5]` **Runtime/Scene/SceneManager.h**
- `[5]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[5]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[5]` **Runtime/Platforms/D3D12/D3D12RootSignature.h**
- `[5]` **Editor/Panel/EditorMainBar.cpp**
- `[5]` **Assets/Shader/include/Core.hlsl**
- `[4]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[4]` **Runtime/Platforms/D3D12/D3D12RootSignature.cpp**
- `[4]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[4]` **Runtime/Platforms/D3D12/d3dUtil.cpp**
- `[4]` **Assets/Shader/GPUCulling.hlsl**
- `[3]` **Runtime/Renderer/RenderContext.cpp**
- `[3]` **Runtime/Renderer/RenderSorter.h**
- `[3]` **Runtime/Resources/Asset.h**
- `[3]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[2]` **run_ai_analysis.bat**
- `[2]` **Editor/EditorGUIManager.h**
- `[2]` **Editor/EditorSettings.h**
- `[2]` **Runtime/CoreAssert.h**
- `[2]` **Runtime/EngineCore.h**
- `[2]` **Runtime/PreCompiledHeader.h**

## Evidence & Implementation Details

### File: `Runtime/MaterialLibrary/MaterialArchetypeRegistry.h`
```cpp
namespace EngineCore
{
    class MaterialArchetypeRegistry
    {
    public:
        static MaterialArchetypeRegistry& GetInstance()
        {
            static MaterialArchetypeRegistry instance;
            return instance;
        }

        bool RegisMaterial(const std::string name, const MaterialLayout& layout)
        {
            layoutMap[name] = layout;
            return true;
        }

        MaterialLayout GetArchytypeLayout(const std::string& name)
        {
            if (layoutMap.count(name) > 0) 
            {
                return layoutMap[name];
            }
            return MaterialLayout();
        }

        
        std::unordered_map<std::string, MaterialLayout> layoutMap; 
    };
```

### File: `Runtime/MaterialLibrary/MaterialInstance.h`
```cpp
namespace EngineCore
{
    class MaterialInstance
    {
    public:
        MaterialInstance(const MaterialLayout& layout)
            :m_Layout(layout)
        {
            m_DataBlob.resize(m_Layout.GetSize());    
        }
        
        void SetValue(const std::string& name, void* data, uint32_t size)
        {
            uint32_t offset = m_Layout.GetPropertyOffset(name);
            memcpy(m_DataBlob.data() + offset, data, size);
        }

        std::vector<uint8_t> GetInstanceData(){ return m_DataBlob; }
        inline void SetInstanceData(const std::vector<uint8_t>& data){ m_DataBlob = data; }
        uint32_t GetSize(){return m_Layout.GetSize();}
        inline MaterialLayout GetLayout(){return m_Layout;};
        inline void SetLayout(const MaterialLayout& layout){ m_Layout = layout;};
        
    private:
        MaterialLayout m_Layout;
        std::vector<uint8_t> m_DataBlob;
        bool m_Dirty = true;
    };
```

### File: `Runtime/MaterialLibrary/MaterialLayout.h`
```cpp
    };

    struct MaterialPropertyLayout
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
        ShaderVariableType type;
    };
```
...
```cpp
        void AddTextureToBlockOffset(const std::string& name, uint32_t offset)
        {
            ASSERT(textureToBlockIndexMap.count(name) < 0);
            textureToBlockIndexMap[name] = offset;
            
        }

        uint32_t GetPropertyOffset(const std::string& name)
        {
```
...
```cpp
        };

        uint32_t GetSize(){ return m_TotalSize;}
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

### File: `Runtime/Serialization/MaterialLoader.h`
```cpp
namespace EngineCore
{
    struct alignas(16) MetaMaterialHeader
    {
        bool     enable;
        uint64_t shaderID;
    };
```
...
```cpp
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
            uint64_t assetPathID = AssetRegistry::GetInstance()->GetAssetIDFromPath(shaderPath);
            ResourceHandle<Shader> shaderHandle(assetPathID);
                result.dependencyList.emplace_back
                (
                    assetPathID,
                    AssetType::Shader,
                    nullptr
                );

            Material* mat = new Material(shaderHandle);
            mat->SetAssetCreateMethod(AssetCreateMethod::Serialization);
            mat->SetPath(relativePath);
            mat->SetAssetID(AssetIDGenerator::NewFromFile(relativePath));
            mat->isBindLessMaterial = isBindless;
            mat->archyTypeName = archyTypeName;

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
                        [=](){
                            ResourceHandle<Texture> texID(textureDependency.ASSETID);
                            mat->SetTexture(texName, texID);
                        }
                    );
            }

            mat->matInstance = std::make_unique<MaterialInstance>(MaterialArchetypeRegistry::GetInstance().GetArchytypeLayout(mat->archyTypeName));

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

### File: `Runtime/Graphics/Material.h`
```cpp
namespace EngineCore
{
    class Material : public Resource
    {
    public:
        bool isDirty = true;
        bool isBindLessMaterial = false;
        string archyTypeName = "";
        std::unique_ptr<MaterialInstance> matInstance;
        ResourceHandle<Shader> mShader;
        unordered_map<string, IGPUTexture*> textureData;
        unordered_map<std::string, ResourceHandle<Texture>> textureHandleMap;
        
        //MaterialData m_MaterialData;
        //void LoadFromMaterialData(const MaterialData& data);

        Material() = default;
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

        // only for serialization
        void SetTexture(const string& name, uint64_t asset)
        {
            ResourceHandle<Texture> texHandle;
            texHandle.mAssetID = AssetID(asset);
            textureHandleMap[name] = texHandle;
        }

        inline MaterailRenderState GetMaterialRenderState() const { return mRenderState;};
        MaterailRenderState mRenderState;
        BufferAllocation materialAllocation;
    private:
        void SetUpRenderState();
        void GetTextureInfoFromShaderReflection();
    };
}
```

### File: `Runtime/Scene/BistroSceneLoader.h`
```cpp
    class Scene;

    class BistroSceneLoader {
    public:
        static Scene* Load(const std::string& path);
        static void SaveToCache(Scene* scene, const std::string& path);
        static Scene* LoadFromCache(const std::string& path);
        static ResourceHandle<Material> commonMatHandle;
        
    private:
        Scene* LoadInternal(const std::string& path);
        void ProcessNode(const tinygltf::Node& node, const tinygltf::Model& model, GameObject* parent, Scene* targetScene);
        void ProcessMesh(int meshIndex, const tinygltf::Model& model, GameObject* go, Scene* targetScene);
        void ProcessMaterials(const tinygltf::Model& model);
        void ProcessShaders();
        void ProcessTexture(const tinygltf::Model& model);
        void CreateDefaultResources();
        void AttachMaterialToGameObject(GameObject* gameObject, int materialIndex);
        AssetID GetTextureAssetID(const tinygltf::Model& model, int textureIndex);
        std::map<int, std::vector<std::pair<ResourceHandle<Mesh>, int>>> m_MeshCache;
        std::vector<AssetID> m_ImageIndexToID;
        std::vector<ResourceHandle<Material>> m_MaterialMap;
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

### File: `Runtime/Renderer/Renderer.h`
```cpp
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
                PROFILER_EVENT_END("RenderThread::ProcessEditorGUI");
#endif


                RenderAPI::GetInstance()->RenderAPIPresentFrame();

                CpuEvent::RenderThreadSubmited().Signal();

                if (hasResize)
                {
                    RenderAPI::GetInstance()->RenderAPIWindowResize(pendingResize);
                    hasResize = false;
                    pendingResize = { 0,0 };
                }

            }
        };
        

    private:
        SPSCRingBuffer<16384> mRenderBuffer;
```

### File: `Runtime/GameObject/MeshRenderer.h`
```cpp
        virtual const char* GetScriptName() const override { return "MeshRenderer"; }
        
        void SetUpMaterialPropertyBlock();

        inline Material* GetSharedMaterial()
        { 
            return mShardMatHandler.IsValid() ? mShardMatHandler.Get() : nullptr;
        };
```
...
```cpp
        Material* GetOrCreateMatInstance();
        // 
        ResourceHandle<Material> GetMaterial();
        inline bool HasMaterialOverride() { return mInstanceMatHandler.IsValid(); }
```

### File: `Runtime/Graphics/GPUSceneManager.h`
```cpp
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