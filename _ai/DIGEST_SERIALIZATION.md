# Architecture Digest: SERIALIZATION
> Auto-generated. Focus: Runtime/Serialization, AssetSerialization, BaseTypeSerialization, ComponentFactory, JsonSerializer, MetaData, MetaFactory, MetaLoader, AssetHeader, StreamHelper, MeshRenderer, MeshFilter, Material, ModelData

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 序列化涉及AssetHeader、StreamHelper与各类Loader。
- 关注二进制格式、版本兼容与加载时的依赖解析。

## Key Files Index
- `[34]` **Runtime/Serialization/MaterialLoader.h** *(Content Included)*
- `[34]` **Runtime/Serialization/SceneLoader.h** *(Content Included)*
- `[34]` **Runtime/Serialization/StreamHelper.h** *(Content Included)*
- `[33]` **Runtime/Serialization/AssetHeader.h** *(Content Included)*
- `[32]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[30]` **Runtime/GameObject/MeshRenderer.cpp** *(Content Included)*
- `[27]` **Runtime/GameObject/MeshFilter.h** *(Content Included)*
- `[27]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[27]` **Runtime/MaterialLibrary/MaterialInstance.h** *(Content Included)*
- `[27]` **Runtime/MaterialLibrary/MaterialLayout.h** *(Content Included)*
- `[26]` **Runtime/GameObject/MeshFilter.cpp** *(Content Included)*
- `[26]` **Runtime/MaterialLibrary/MaterialArchytype.h** *(Content Included)*
- `[25]` **Runtime/Graphics/Material.cpp** *(Content Included)*
- `[25]` **Runtime/MaterialLibrary/MaterialArchetypeRegistry.h** *(Content Included)*
- `[23]` **Runtime/Graphics/GPUSceneManager.cpp** *(Content Included)*
- `[20]` **Runtime/Entry.cpp** *(Content Included)*
- `[20]` **Runtime/Serialization/MeshLoader.h** *(Content Included)*
- `[18]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[17]` **Runtime/MaterialLibrary/StandardPBR.h** *(Content Included)*
- `[16]` **Runtime/Renderer/BatchManager.h** *(Content Included)*
- `[16]` **Runtime/Scene/Scene.h**
- `[15]` **Runtime/Scene/Scene.cpp**
- `[15]` **Runtime/Serialization/DDSTextureLoader.h**
- `[13]` **Editor/Panel/EditorMainBar.cpp**
- `[12]` **Runtime/Core/Game.cpp**
- `[12]` **Runtime/Renderer/BatchManager.cpp**
- `[12]` **Runtime/Renderer/RenderContext.cpp**
- `[12]` **Runtime/Renderer/RenderEngine.cpp**
- `[12]` **Runtime/Renderer/Renderer.h**
- `[12]` **Runtime/Scene/SceneManager.cpp**
- `[12]` **Runtime/Serialization/TextureLoader.h**
- `[11]` **Runtime/Core/PublicStruct.h**
- `[11]` **Runtime/MaterialLibrary/StandardPBR.cpp**
- `[9]` **Runtime/Renderer/Culling.cpp**
- `[9]` **Runtime/Scene/BistroSceneLoader.h**
- `[9]` **Editor/Panel/EditorMainBar.h**
- `[7]` **Runtime/Graphics/GPUSceneManager.h**
- `[7]` **Runtime/Renderer/RenderAPI.h**
- `[7]` **Runtime/Renderer/RenderCommand.h**
- `[7]` **Runtime/Renderer/RenderStruct.h**
- `[6]` **Runtime/Scene/SceneManager.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[6]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[6]` **Assets/Shader/BlitShader.hlsl**
- `[6]` **Assets/Shader/SimpleTestShader.hlsl**
- `[6]` **Assets/Shader/StandardPBR.hlsl**
- `[6]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[5]` **Runtime/Renderer/Renderer.cpp**
- `[5]` **Runtime/Renderer/RenderSorter.h**
- `[5]` **Runtime/Resources/AssetRegistry.cpp**
- `[5]` **Runtime/Resources/AssetTypeTraits.h**
- `[5]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[5]` **Runtime/Platforms/D3D12/D3D12RootSignature.h**
- `[5]` **Assets/Shader/include/Core.hlsl**
- `[4]` **Runtime/GameObject/ComponentType.h**
- `[4]` **Runtime/GameObject/GameObject.h**
- `[4]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[4]` **Runtime/Platforms/D3D12/D3D12RootSignature.cpp**

## Evidence & Implementation Details

### File: `Runtime/Serialization/MaterialLoader.h`
```cpp
#include "Settings/ProjectSettings.h"

namespace EngineCore
{
    
};
```

### File: `Runtime/Serialization/SceneLoader.h`
```cpp
            Scene* scene = new Scene();
            std::vector<SceneSerializedNode> allnode;
            StreamHelper::ReadVector(in, allnode);
            std::unordered_map<uint32_t, GameObject*> gameObjectMap;
            
            for(int i = 0; i < allnode.size(); i++)
            {
                auto& nodeData = allnode[i];
                
                std::string nodeName = nodeData.name;
                GameObject* go = scene->CreateGameObject(nodeName.empty() ? "Node" : nodeName);

                go->transform->SetLocalPosition(nodeData.position);
                go->transform->SetLocalQuaternion(nodeData.rotation);
                go->transform->SetLocalScale(nodeData.scale);
                if(nodeData.parentIndex != -1)
                {
                    ASSERT(gameObjectMap.count(nodeData.parentIndex) > 0);
                    go->SetParent(gameObjectMap[nodeData.parentIndex]);
                }
                gameObjectMap[i] = go;

                //todo 加入材质的异步加载：
                if(nodeData.meshID != 0)
                {
                    MeshFilter* filter = go->AddComponent<MeshFilter>();
                    filter->mMeshHandle = ResourceManager::GetInstance()->LoadAssetAsync<Mesh>(nodeData.meshID, [=]() 
                        {
                            filter->OnLoadResourceFinished();
                        });
                    
                    // 加载并设置 Material
                    if(nodeData.materialID != 0)
                    {
                        MeshRenderer* renderer = go->AddComponent<MeshRenderer>();
                        AssetRegistry::GetInstance()->GetAssetPath(nodeData.materialID);
                        // todo  还没做
                        //renderer->SetSharedMaterial(ResourceManager::GetInstance()->LoadAsset<Material>(nodeData.materialID));
                        //renderer->TryAddtoBatchManager();
                    }
                }

            }
```
...
```cpp
            header.assetID =id;
            header.type = 0;
            StreamHelper::Write(out, header);

            std::vector<SceneSerializedNode> linearNode;
            std::unordered_map<GameObject*, uint32_t> gameObjectMap;
            for(int i = 0; i < scene->rootObjList.size(); i++)
            {
                GameObject* gameObject = scene->rootObjList[i];
                
                SerilizedNode(gameObject, gameObjectMap, linearNode);
            }
```

### File: `Runtime/Serialization/StreamHelper.h`
```cpp
namespace EngineCore
{
    class StreamHelper
    {
    public:
        // 写pod data
        template<typename T>
        static void Write(std::ofstream& out, const T& value)
        {
            out.write(reinterpret_cast<const char*>(&value), sizeof(T));
        }

        // 读pod data
        template<typename T>
        static void Read(std::ifstream& in, T& value)
        {
            in.read(reinterpret_cast<char*>(&value), sizeof(T));
        };

        template<typename T>
        static void WriteVector(std::ofstream& out, const std::vector<T>& vec)
        {
            uint32_t size = (uint32_t) vec.size();
            out.write((char*)&size, sizeof(uint32_t));
            if(size > 0)out.write((char*)vec.data(), size * sizeof(T));
        }

        // 直接整段copy
        template<typename T>
        static void ReadVector(std::ifstream& in, std::vector<T>& vec)
        {
            uint32_t size = 0;
            in.read((char*)&size, sizeof(uint32_t));
            vec.resize(size);
            if(size > 0)in.read((char*)vec.data(), sizeof(T) * size);
        }

        static void ReadString(std::ifstream& in, std::string& str)
        {
            uint32_t len = 0;
            in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
            if(len > 0)
            {
                str.resize(len);
                in.read(reinterpret_cast<char*>(&str[0]), len);
            }
            else
            {
                str.clear();
            }
        }

        static void WriteString(std::ofstream& out, const std::string& str)
        {
            // ASSERT(str.size() > 0); // Allow empty strings
            uint32_t len = (uint32_t)str.size();
            out.write((char*)&len, sizeof(uint32_t));
            if (len > 0) out.write((char*)str.c_str(), len);
        }
    };

}
```

### File: `Runtime/Serialization/AssetHeader.h`
```cpp
    // type 1 = Texture
    // type 2 = Mesh
    struct AssetHeader
    {
        char magic[4] = {'E', 'N', 'G', 'N'};
        uint32_t version = 1;
        uint32_t assetID = 0;
        uint32_t type = 0; 
    };
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
```

### File: `Runtime/GameObject/MeshFilter.h`
```cpp
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
```

### File: `Runtime/Graphics/Material.h`
```cpp
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
        
        MaterialData m_MaterialData;
        void LoadFromMaterialData(const MaterialData& data);

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

        inline MaterailRenderState GetMaterialRenderState() const { return mRenderState;};
        MaterailRenderState mRenderState;
        BufferAllocation materialAllocation;
    private:
        void SetUpRenderState();
        void GetTextureInfoFromShaderReflection();
    };
}
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

        void* GetData(){return m_DataBlob.data();}
        uint32_t GetSize(){return m_Layout.GetSize();}
        inline MaterialLayout GetLayout(){return m_Layout;};
    private:
        unordered_map<std::string, IGPUTexture*> textureMap;
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
```
...
```cpp
        };

        uint32_t GetSize(){ return m_TotalSize;}
```

### File: `Runtime/MaterialLibrary/MaterialArchytype.h`
```cpp
    // 对应 MaterialLayout::GetDefaultPBRLayout 的内存布局
    // 必须保持 16 字节对齐
    struct MaterialConstants
    {
        Vector4 DiffuseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        Vector4 SpecularColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
        float Roughness = 0.5f;
        float Metallic = 0.0f;
        Vector2 TilingFactor = Vector2(1.0f, 1.0f);
        float DiffuseTextureIndex = 0.0f; 
        Vector3 Padding = Vector3(0.0f, 0.0f, 0.0f);
    };
```

### File: `Runtime/MaterialLibrary/MaterialArchetypeRegistry.h`
```cpp
namespace EngineCore
{
    class MaterialArchetypeRegistry
    {
    public:
        static MaterialArchetypeRegistry& Get()
        {
            static MaterialArchetypeRegistry instance;
            return instance;
        }
    };
```

### File: `Runtime/Serialization/MeshLoader.h`
```cpp
            mesh->SetAssetCreateMethod(AssetCreateMethod::Serialization);
            mesh->SetAssetID(AssetIDGenerator::NewFromFile(path));
            StreamHelper::Read(in, mesh->bounds);
            StreamHelper::ReadVector(in, mesh->vertex);
            StreamHelper::ReadVector(in, mesh->index);
            return mesh;
        }

        void SaveMeshToBin(const Mesh* mesh, const std::string& relativePath, uint32_t id)
        {
```
...
```cpp
            header.assetID =id;
            header.type = 2;
            StreamHelper::Write(out, header);

            StreamHelper::Write(out, mesh->bounds);
            StreamHelper::WriteVector(out, mesh->vertex);
            StreamHelper::WriteVector(out, mesh->index);
        }

    };
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

### File: `Runtime/Renderer/BatchManager.h`
```cpp
        Mesh* mesh;
        DrawIndirectContext() = default;
        DrawIndirectContext(Material* mat, Mesh* mesh): material(mat), mesh(mesh){}
```
...
```cpp
        }

        void TryAddBatchCount(MeshRenderer* meshRenderer);
        void TryDecreaseBatchCount(MeshRenderer* meshRenderer);

        void TryAddBatchCount(MeshFilter* meshFilter);
        void TryDecreaseBatchCount(MeshFilter* meshFilter);
        static std::unordered_map<uint64_t, int> BatchMap;
        static std::unordered_map<uint64_t, DrawIndirectParam> drawIndirectParamMap;
        static std::unordered_map<uint64_t, DrawIndirectContext> drawIndirectContextMap;

        std::vector<RenderProxy> GetAvaliableRenderProxyList(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        static uint64_t GetBatchHash(MeshRenderer* meshRenderer, MeshFilter* meshFilter, uint32_t layer); 

        vector<DrawIndirectArgs> GetBatchInfo();
    private:
        void TryAddBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        void TryDecreaseBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        static void Create();
        static BatchManager* s_Instance;

    };
}
```