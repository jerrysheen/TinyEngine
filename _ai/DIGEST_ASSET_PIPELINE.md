# Architecture Digest: ASSET_PIPELINE
> Auto-generated. Focus: Runtime/Resources, Runtime/Serialization, Meta, Asset, Importer, Texture, Material, Shader, Mesh

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 资源导入、序列化、元数据与运行时缓存关系。
- 关注资源句柄、依赖管理与加载策略。

## Key Files Index
- `[51]` **Runtime/Serialization/MetaData.h** *(Content Included)*
- `[49]` **Runtime/Serialization/MetaLoader.h** *(Content Included)*
- `[48]` **Runtime/Serialization/AssetSerialization.h** *(Content Included)*
- `[42]` **Runtime/Resources/Asset.h** *(Content Included)*
- `[41]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[39]` **Assets/Shader/SimpleTestShader.hlsl** *(Content Included)*
- `[38]` **Runtime/Serialization/MetaFactory.h** *(Content Included)*
- `[36]` **Assets/Shader/BlitShader.hlsl** *(Content Included)*
- `[35]` **Runtime/Graphics/MaterialLayout.h** *(Content Included)*
- `[33]` **Runtime/Graphics/Mesh.h** *(Content Included)*
- `[33]` **Assets/Shader/include/Core.hlsl** *(Content Included)*
- `[32]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[30]` **Runtime/Graphics/Shader.h** *(Content Included)*
- `[30]` **Runtime/Serialization/BaseTypeSerialization.h**
- `[29]` **Runtime/Graphics/MaterialInstance.h**
- `[29]` **Runtime/Graphics/Texture.h**
- `[29]` **Assets/Shader/StandardPBR.hlsl**
- `[28]` **Editor/Panel/EditorMainBar.h**
- `[27]` **Runtime/GameObject/MeshFilter.h**
- `[27]` **Runtime/Graphics/MeshUtils.h**
- `[27]` **Runtime/Graphics/RenderTexture.h**
- `[27]` **Runtime/Resources/ResourceManager.h**
- `[27]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[27]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[26]` **Runtime/Graphics/ComputeShader.h**
- `[25]` **Runtime/Graphics/GPUTexture.h**
- `[22]` **Runtime/Core/PublicStruct.h**
- `[22]` **Runtime/Renderer/RenderCommand.h**
- `[22]` **Runtime/Resources/Resource.h**
- `[20]` **Assets/Shader/GPUCulling.hlsl**
- `[19]` **Runtime/Renderer/RenderAPI.h**
- `[19]` **Runtime/Serialization/JsonSerializer.h**
- `[17]` **Runtime/Resources/ResourceHandle.h**
- `[17]` **Runtime/Scene/SceneManager.h**
- `[16]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[14]` **Runtime/Renderer/Renderer.h**
- `[13]` **Runtime/Renderer/RenderStruct.h**
- `[12]` **Runtime/Serialization/ComponentFactory.h**
- `[12]` **Runtime/Platforms/D3D12/d3dx12.h**
- `[11]` **Runtime/Renderer/BatchManager.h**
- `[11]` **Runtime/Scene/Scene.h**
- `[11]` **Runtime/Platforms/D3D12/D3D12RootSignature.h**
- `[10]` **Runtime/Graphics/GPUSceneManager.h**
- `[10]` **Runtime/Graphics/IGPUResource.h**
- `[10]` **Runtime/Renderer/RenderSorter.h**
- `[10]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[9]` **Runtime/Scene/BistroSceneLoader.h**
- `[8]` **Runtime/Core/PublicEnum.h**
- `[8]` **Runtime/GameObject/Camera.h**
- `[7]` **Runtime/Graphics/GeometryManager.h**
- `[7]` **Runtime/Settings/ProjectSettings.h**
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[5]` **premake5.lua**
- `[5]` **Runtime/Core/Profiler.h**
- `[5]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12DescManager.h**
- `[4]` **Runtime/GameObject/ComponentType.h**
- `[4]` **Runtime/GameObject/GameObject.h**
- `[4]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h**
- `[4]` **Runtime/Platforms/D3D12/D3D12PSO.h**

## Evidence & Implementation Details

### File: `Runtime/Serialization/MetaData.h`
```cpp
{

    class Texture;
    class Material;
    struct MetaData
    {
        string path;
        AssetType assetType;
        // k,v = <ResouceName, MetaData>
        std::unordered_map<std::string, MetaData> dependentMap;
        MetaData() = default;
        MetaData(const std::string& path, AssetType type)
            : path(path), assetType(type) {};
    };
```
...
```cpp
    };

    struct MaterialMetaData : MetaData 
    {
		string shaderPath;       
        unordered_map<string, float> floatData;
        unordered_map<string, Vector2> vec2Data;
        unordered_map<string, Vector3> vec3Data;
        unordered_map<string, Matrix4x4> matrix4x4Data;
        // 无意义的Texture*，只是为了和MaterialData做方便的同步而已。
        unordered_map<string, Texture*> textureData;
    };
```

### File: `Runtime/Serialization/MetaLoader.h`
```cpp
namespace EngineCore
{
    class MetaLoader
    {
    public:
        static MaterialMetaData* LoadMaterialMetaData(const std::string& path);
        static ShaderVariableType GetShaderVaribleType(uint32_t size);
        static TextureMetaData* LoadTextureMetaData(const std::string& path);
        static ModelMetaData* LoadModelMetaData(const std::string& path);
        template<typename T>
        static MetaData* LoadMetaData(const std::string& path)
        {
            MetaData* metaData = new MetaData();
            metaData->path = path;
            return metaData;
            // do nothing, for example shader..
        };

        

        template<typename T>
        static T* LoadMeta(const std::string& path)
        {
            std::ifstream file(path);
            
            if(!file.is_open())
            {
                throw std::runtime_error("Can't open Meta File" + path);
            }

            json j = json::parse(file);
            file.close();
            
            return j.get<T>();
        }

    };
```

### File: `Runtime/Serialization/AssetSerialization.h`
```cpp


namespace EngineCore
{
    // Meta
    inline void to_json(json& j, const EngineCore::MetaData& v)
    {
        j = {  
                {"Path", v.path}, 
                {"AssetType", v.assetType}, 
                {"Dependencies", v.dependentMap}, 
            };
    }

    inline void from_json(const json& j, EngineCore::MetaData& v)
    {
        j.at("Path").get_to(v.path);
        j.at("AssetType").get_to(v.assetType);
        j.at("Dependencies").get_to(v.dependentMap);
    }


    // TextureMetaData
    inline void to_json(json& j, const EngineCore::TextureMetaData& v)
    {
        j = {  
                {"Path", v.path}, 
                {"AssetType", v.assetType}, 
                {"Dependencies", v.dependentMap}, 
                {"Dimension", v.dimension}, 
                {"Format", v.format},
                {"Width", v.width},
                {"Height", v.height}
            };
    }

    inline void from_json(const json& j, EngineCore::TextureMetaData& v)
    {
        j.at("Path").get_to(v.path);
        j.at("AssetType").get_to(v.assetType);
        j.at("Dependencies").get_to(v.dependentMap);
        j.at("Dimension").get_to(v.dimension);
        j.at("Format").get_to(v.format);
        j.at("Width").get_to(v.width);
        j.at("Height").get_to(v.height);
    }

    // MaterialMetaData
    inline void to_json(json& j, const EngineCore::MaterialMetaData& v)
    {
        j = {  
                {"Path", v.path}, 
                {"AssetType", v.assetType}, 
                {"Dependencies", v.dependentMap}, 
                {"FloatData", v.floatData}, 
                {"Vec2Data", v.vec2Data},
                {"Vec3Data", v.vec3Data},
                {"Matrix4x4Data", v.matrix4x4Data}
            };
    }
```

### File: `Runtime/Resources/Asset.h`
```cpp
{
    //todo： 应该是一个永久的hash，保证每次的资源能对应上
    struct AssetID
    {
        uint64_t value = 0;
        explicit operator bool() const {return value != 0;};
        operator uint32_t() const { return value;}
        bool IsValid() const {return value != 0;};
        inline void Reset() { value = 0; };
        AssetID() = default;
        AssetID(uint64_t value) :value(value) {};
        // 添加相等比较运算符
        bool operator==(const AssetID& other) const {
            // 假设 AssetID 有一个 id 成员变量
            return this->value == other.value;
        }

        bool operator!=(const AssetID& other) const {
            return !(*this == other);
        }
    };
```
...
```cpp
    };

    enum class AssetType
    {
        Default = 0,
        Texture2D = 1,
        Mesh = 2,
        Material = 3,
        SkinMesh = 4,
        Animation = 5,
        Shader = 6,
        Runtime = 7,
    };
```
...
```cpp
namespace std {
    template<>
    struct hash<EngineCore::AssetID> {
        size_t operator()(const EngineCore::AssetID& a) const noexcept {
            return hash<uint64_t>{}(a.value);
        }
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

### File: `Runtime/Serialization/MetaFactory.h`
```cpp
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
            {
                json compJson = ConvertToJson<GameObject>(go);
                rootObjectArray.push_back(compJson);
            }
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

### File: `Runtime/Graphics/MaterialLayout.h`
```cpp
namespace EngineCore
{
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

### File: `Runtime/Graphics/Mesh.h`
```cpp
    };

    struct MeshBufferAllocation
    {
        IGPUBuffer* buffer = nullptr;
        // 当前数据开始位置， 可以直接绑定
        uint64_t gpuAddress = 0;
        uint64_t offset =0;
        uint64_t size = 0;
        uint32_t stride = 0;
        bool isValid = false;
        struct MeshBufferAllocation() = default;
        struct MeshBufferAllocation(IGPUBuffer* buffer, uint64_t gpuAddress, uint64_t offset, uint64_t size, uint64_t stride)
            :buffer(buffer), gpuAddress(gpuAddress), offset(offset), size(size), stride(stride)
        {
            isValid = true;
        }
    };
```
...
```cpp

        Mesh() = default;
        Mesh(MetaData* metaData);
        Mesh(Primitive primitiveType);
        MeshBufferAllocation* vertexAllocation;
        MeshBufferAllocation* indexAllocation;
        void UploadMeshToGPU();

        AABB bounds;
        std::vector<Vertex> vertex;
        std::vector<int> index;
        std::vector<InputLayout> layout;
    private:
        void ProcessNode(aiNode* node, const aiScene* scene);
        void LoadAiMesh(const string& path);
        void ProcessMesh(aiMesh* aiMesh, const aiScene* scene);

    };

}
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
        virtual json SerializedFields() const override {
            return json{
                {"MatHandle", mShardMatHandler},
            };
        }
        
        virtual void DeserializedFields(const json& data) override;
        
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
```