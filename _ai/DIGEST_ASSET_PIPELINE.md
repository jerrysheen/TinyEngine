# Architecture Digest: ASSET_PIPELINE
> Auto-generated. Focus: Runtime/Resources, Runtime/Serialization, Asset, Importer, Texture, Material, Shader, Mesh, MeshLoader, StreamHelper

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
- `[69]` **Runtime/Serialization/MeshLoader.h** *(Content Included)*
- `[46]` **Runtime/Resources/AssetTypeTraits.h** *(Content Included)*
- `[45]` **Runtime/Serialization/DDSTextureLoader.h** *(Content Included)*
- `[42]` **Runtime/Resources/Asset.h** *(Content Included)*
- `[38]` **Assets/Shader/SimpleTestShader.hlsl** *(Content Included)*
- `[37]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[37]` **Runtime/Resources/AssetRegistry.h** *(Content Included)*
- `[36]` **Runtime/Serialization/AssetHeader.h** *(Content Included)*
- `[36]` **Assets/Shader/BlitShader.hlsl** *(Content Included)*
- `[34]` **Runtime/Serialization/StreamHelper.h** *(Content Included)*
- `[33]` **Runtime/Graphics/MaterialLayout.h** *(Content Included)*
- `[32]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[32]` **Runtime/Graphics/MaterialData.h** *(Content Included)*
- `[32]` **Runtime/Serialization/TextureLoader.h** *(Content Included)*
- `[30]` **Runtime/Graphics/Mesh.h** *(Content Included)*
- `[30]` **Assets/Shader/include/Core.hlsl**
- `[29]` **Runtime/Graphics/MaterialInstance.h**
- `[29]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[28]` **Runtime/Serialization/SceneLoader.h**
- `[28]` **Assets/Shader/StandardPBR.hlsl**
- `[27]` **Runtime/GameObject/MeshFilter.h**
- `[27]` **Runtime/Graphics/MeshUtils.h**
- `[27]` **Runtime/Graphics/RenderTexture.h**
- `[27]` **Runtime/Graphics/Shader.h**
- `[27]` **Runtime/Graphics/Texture.h**
- `[27]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[27]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[26]` **Runtime/Graphics/ComputeShader.h**
- `[25]` **Runtime/Graphics/GPUTexture.h**
- `[22]` **Runtime/Core/PublicStruct.h**
- `[22]` **Runtime/Renderer/RenderCommand.h**
- `[21]` **Runtime/Resources/ResourceManager.h**
- `[21]` **Editor/Panel/EditorMainBar.h**
- `[20]` **Assets/Shader/GPUCulling.hlsl**
- `[19]` **Runtime/Renderer/RenderAPI.h**
- `[18]` **Runtime/Scene/BistroSceneLoader.h**
- `[17]` **Runtime/Resources/Resource.h**
- `[17]` **Runtime/Resources/ResourceHandle.h**
- `[17]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[16]` **Runtime/Scene/SceneManager.h**
- `[14]` **Runtime/Renderer/Renderer.h**
- `[13]` **Runtime/Renderer/RenderStruct.h**
- `[12]` **Runtime/Resources/IResourceLoader.h**
- `[12]` **Runtime/Platforms/D3D12/d3dx12.h**
- `[11]` **Runtime/Renderer/BatchManager.h**
- `[11]` **Runtime/Scene/Scene.h**
- `[11]` **Runtime/Platforms/D3D12/D3D12RootSignature.h**
- `[10]` **Runtime/Graphics/GPUSceneManager.h**
- `[10]` **Runtime/Graphics/IGPUResource.h**
- `[10]` **Runtime/Renderer/RenderSorter.h**
- `[10]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[8]` **Runtime/Core/PublicEnum.h**
- `[8]` **Runtime/GameObject/Camera.h**
- `[7]` **Runtime/Graphics/GeometryManager.h**
- `[7]` **Runtime/Settings/ProjectSettings.h**
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[5]` **premake5.lua**
- `[5]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12DescManager.h**
- `[4]` **Runtime/GameObject/ComponentType.h**

## Evidence & Implementation Details

### File: `Runtime/Serialization/MeshLoader.h`
```cpp
namespace EngineCore
{
    class MeshLoader : public IResourceLoader
    {
    public:
        virtual ~MeshLoader() = default;
        virtual Resource* Load(const std::string& relativePath) override
        {
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream in(path, std::ios::binary);
            in.seekg(sizeof(AssetHeader));

            Mesh* mesh = new Mesh();
            mesh->SetAssetCreateMethod(AssetCreateMethod::Serialization);
            mesh->SetAssetID(AssetIDGenerator::NewFromFile(path));
            StreamHelper::Read(in, mesh->bounds);
            StreamHelper::ReadVector(in, mesh->vertex);
            StreamHelper::ReadVector(in, mesh->index);
            return mesh;
        }

        void SaveMeshToBin(const Mesh* mesh, const std::string& relativePath, uint32_t id)
        {
            ASSERT(mesh && mesh->vertex.size() > 0 && mesh->index.size() > 0);
            std::string binPath = PathSettings::ResolveAssetPath(relativePath);
            std::ofstream out(binPath, std::ios::binary);

            AssetHeader header;
            header.assetID =id;
            header.type = 2;
            StreamHelper::Write(out, header);

            StreamHelper::Write(out, mesh->bounds);
            StreamHelper::WriteVector(out, mesh->vertex);
            StreamHelper::WriteVector(out, mesh->index);
        }

    };
```

### File: `Runtime/Resources/AssetTypeTraits.h`
```cpp
namespace EngineCore
{
    class Mesh;
    class Texture;
    class Material;

    template<typename T> struct AssetTypeTraits { static const AssetType Type = AssetType::Default; };
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

    class DDSTextureLoader : public IResourceLoader
    {
    public:
        virtual ~DDSTextureLoader() = default;
        virtual Resource* Load(const std::string& relativePath) override
        {
            std::string path = PathSettings::ResolveAssetPath(relativePath);
    
            Texture* tex = new Texture();
            tex->SetAssetCreateMethod(AssetCreateMethod::Serialization);
            tex->SetAssetID(AssetIDGenerator::NewFromFile(path));
            
            DDSLoadResult ddsResult = LoadDDSFromFile(relativePath);
            tex->textureDesc.format = ddsResult.format;
            tex->textureDesc.width = ddsResult.width;
            tex->textureDesc.height = ddsResult.height;
            tex->textureDesc.dimension = TextureDimension::TEXTURE2D;
            tex->textureDesc.texUsage = TextureUsage::ShaderResource;
            tex->textureDesc.mipCount = ddsResult.mipMapCount;
            tex->cpuData = ddsResult.pixelData;
            // 计算mip Count
            uint32_t offset = 0;
            tex->textureDesc.mipOffset[0] = 0;  // 第 0 级从 0 开始
            int width = ddsResult.width;
            int height = ddsResult.height;
            // ++i 和 i++在循环体中区别不大， 因为循环跑完才会走++操作，
            // 不过针对迭代器，++i更好， 因为i++相当于要返回一个原值，并且在原来的迭代器上叠加
            for (uint32_t i = 0; i < ddsResult.mipMapCount; ++i)
            {
                // 当前 mip level 的尺寸
                uint32_t mipWidth = std::max(1, width >> i);
                uint32_t mipHeight = std::max(1, height >> i);
                
                // 计算当前 mip level 的字节大小
                uint32_t mipSize = CalculateDXTMipSize(mipWidth, mipHeight, ddsResult.blockSize);
                
                if (i > 0) {
                    tex->textureDesc.mipOffset[i] = offset;  // 记录当前 mip 的 offset
                }
                
                offset += mipSize;  // 累加到下一个 mip level
            }


            return tex;
        }
    
        std::vector<uint8_t> LoadMipData(const std::string& realativePath, int mipCount){}
    
    
        int CalculateDXTMipSize(uint32_t width, uint32_t height, uint32_t blockSize)
        {
            //DXT1 (BC1)：每 4×4 块占 8 字节
            //DXT3 (BC2)：每 4×4 块占 16 字节
            //DXT5 (BC3)：每 4×4 块占 16 字节
            uint32_t blockWidth = (width + 3) / 4;
            uint32_t blockHeight = (height + 3) / 4;
            return blockWidth * blockHeight * blockSize;
```
...
```cpp
        {
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream file(path, std::ios::binary);
            ASSERT(file.is_open());
    
            file.seekg(0, std::ios::end);      // 先移动到文件末尾
            std::streamsize fileSize = file.tellg();  // 获取文件大小
            file.seekg(0, std::ios::beg);      // 再移回文件开始
    
            ASSERT(fileSize > sizeof(DDSHeader));
    
            DDSHeader header;
            file.read(reinterpret_cast<char*>(&header), sizeof(DDSHeader));
    
            if(header.magic != 0x20534444 || header.fileSize != 124)
            {
                ASSERT(false);
            }
```
...
```cpp
            // 提取fourCC字符串来判断
            char fourCCStr[5] = {0};
            std::memcpy(fourCCStr, &fourCC, 4);
            
            if (std::memcmp(fourCCStr, "DXT1", 4) == 0) {
                result.format = TextureFormat::DXT1;  // DXT1
                result.blockSize = 8;
            }
```

### File: `Runtime/Resources/Asset.h`
```cpp

    //todo： 应该是一个永久的hash，保证每次的资源能对应上
    struct AssetID
    {
        uint64_t value = 0;
        explicit operator bool() const {return value != 0;};
        operator uint64_t() const { return value;}
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
        Scene = 8
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

### File: `Runtime/Resources/AssetRegistry.h`
```cpp
namespace EngineCore
{
    class AssetRegistry
    {
    public:
        static void Create();
        static void Destroy();
        static AssetRegistry* GetInstance();
        void RegisterAsset(Resource* resource);
        std::string GetAssetPath(uint64_t id);
        void SaveToDisk(const std::string& manifestPath);
        void LoadFromDisk(const std::string& manifestPath);
    private:
        static AssetRegistry* s_Instacnce;
        std::unordered_map<uint64_t, std::string> assetPathMap;
    };
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

### File: `Runtime/Graphics/MaterialData.h`
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
        Mesh(Primitive primitiveType);
        MeshBufferAllocation* vertexAllocation;
        MeshBufferAllocation* indexAllocation;
        void UploadMeshToGPU();

        AABB bounds;
        std::vector<Vertex> vertex;
        std::vector<int> index;
        std::vector<InputLayout> layout;
        bool isDynamic = true;
        virtual void OnLoadComplete() override { UploadMeshToGPU(); };
```
...
```cpp
        void ProcessNode(aiNode* node, const aiScene* scene);
        void LoadAiMesh(const string& path);
        void ProcessMesh(aiMesh* aiMesh, const aiScene* scene);

    };

}
```