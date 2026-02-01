# Architecture Digest: ASSET_PIPELINE
> Auto-generated. Focus: Runtime/Resources, Runtime/Serialization, Runtime/MaterialLibrary, Asset, AssetRegistry, ResourceManager, Importer, Texture, Material, Shader, Mesh, MeshLoader, TextureLoader, MaterialLoader, SceneLoader, StreamHelper

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 资源导入、序列化、元数据与运行时缓存关系。
- 关注资源句柄、依赖管理与加载策略。

## Key Files Index
- `[81]` **Runtime/Serialization/MaterialLoader.h** *(Content Included)*
- `[72]` **Runtime/Resources/ResourceManager.cpp** *(Content Included)*
- `[69]` **Runtime/Serialization/MeshLoader.h** *(Content Included)*
- `[67]` **Runtime/Serialization/DDSTextureLoader.h** *(Content Included)*
- `[66]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[65]` **Runtime/Resources/AssetRegistry.cpp** *(Content Included)*
- `[61]` **Runtime/Resources/ResourceManager.h** *(Content Included)*
- `[60]` **Runtime/Resources/AssetRegistry.h** *(Content Included)*
- `[57]` **Runtime/Serialization/SceneLoader.h** *(Content Included)*
- `[52]` **Runtime/Serialization/TextureLoader.h** *(Content Included)*
- `[49]` **Runtime/Resources/AssetTypeTraits.h** *(Content Included)*
- `[45]` **Runtime/MaterialLibrary/MaterialLayout.h** *(Content Included)*
- `[42]` **Runtime/Graphics/Material.cpp** *(Content Included)*
- `[42]` **Runtime/Resources/Asset.h** *(Content Included)*
- `[42]` **Assets/Shader/SimpleTestShader.hlsl** *(Content Included)*
- `[41]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[41]` **Runtime/Scene/BistroSceneLoader.h** *(Content Included)*
- `[40]` **Assets/Shader/BlitShader.hlsl** *(Content Included)*
- `[38]` **Runtime/Serialization/ShaderLoader.h** *(Content Included)*
- `[37]` **Runtime/MaterialLibrary/MaterialArchetypeRegistry.h** *(Content Included)*
- `[37]` **Runtime/MaterialLibrary/MaterialInstance.h**
- `[36]` **Runtime/Serialization/AssetHeader.h**
- `[34]` **Runtime/Serialization/StreamHelper.h**
- `[34]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[33]` **Runtime/MaterialLibrary/StandardPBR.h**
- `[33]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[32]` **Runtime/GameObject/MeshRenderer.cpp**
- `[32]` **Runtime/GameObject/MeshRenderer.h**
- `[32]` **Runtime/Graphics/Mesh.cpp**
- `[32]` **Assets/Shader/StandardPBR.hlsl**
- `[31]` **Runtime/Scene/SceneManager.cpp**
- `[31]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[30]` **Runtime/Graphics/Mesh.h**
- `[30]` **Assets/Shader/include/Core.hlsl**
- `[27]` **Runtime/GameObject/MeshFilter.h**
- `[27]` **Runtime/Graphics/MeshUtils.h**
- `[27]` **Runtime/Graphics/RenderTexture.h**
- `[27]` **Runtime/Graphics/Shader.cpp**
- `[27]` **Runtime/Graphics/Shader.h**
- `[27]` **Runtime/Graphics/Texture.h**
- `[27]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[26]` **Runtime/Graphics/ComputeShader.cpp**
- `[26]` **Runtime/Graphics/ComputeShader.h**
- `[26]` **Runtime/Graphics/GPUSceneManager.cpp**
- `[25]` **Runtime/GameObject/MeshFilter.cpp**
- `[25]` **Runtime/Graphics/GPUTexture.h**
- `[25]` **Runtime/Graphics/MeshUtils.cpp**
- `[25]` **Runtime/Graphics/RenderTexture.cpp**
- `[25]` **Runtime/Graphics/Texture.cpp**
- `[24]` **Runtime/MaterialLibrary/StandardPBR.cpp**
- `[24]` **Editor/Panel/EditorMainBar.h**
- `[24]` **Assets/Shader/GPUCulling.hlsl**
- `[22]` **Runtime/Core/PublicStruct.h**
- `[22]` **Runtime/Renderer/RenderCommand.h**
- `[22]` **Runtime/Renderer/Renderer.cpp**
- `[22]` **Runtime/Resources/ResourceHandle.h**
- `[21]` **Runtime/Core/Game.cpp**
- `[20]` **Runtime/Entry.cpp**
- `[20]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[19]` **Runtime/Renderer/RenderAPI.h**

## Evidence & Implementation Details

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

    struct alignas(16) MetaTextureToBindlessBlockIndex
    {
        char name[50];
        uint32_t offset;
    };
```
...
```cpp
            in.seekg(sizeof(AssetHeader));
            bool isBindless = false;
            StreamHelper::Read(in, isBindless);

            std::string archyTypeName;
            StreamHelper::ReadString(in, archyTypeName);
            if (archyTypeName.empty())
            {
                archyTypeName = "StandardPBR";
            }
```
...
```cpp
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

### File: `Runtime/Resources/ResourceManager.cpp`
```cpp
        m_Loaders[AssetType::Shader] = new ShaderLoader();
        defaultMesh = new Mesh(Primitive::Cube);
        EnsureDefaultShader();
        EnsureDefaultTexture();
    }

    void ResourceManager::Update()
    {
```
...
```cpp
                    {
                    case AssetType::Texture2D:
                        ResourceManager::GetInstance()->LoadAssetAsync<Texture>(dependency.id, dependency.onLoaded, loadResult.task);
                        break;
                    case AssetType::Shader:
                        ResourceManager::GetInstance()->LoadAssetAsync<Shader>(dependency.id, dependency.onLoaded, loadResult.task);
                        break;
                    default:
                        ASSERT(false);
                    }
                }
                TryFinalize(task);
            };
        }
    }

    void ResourceManager::WorkThreadLoad()
    {
```
...
```cpp
    Resource* ResourceManager::GetDefaultResource(AssetType fileType)
    {
        ASSERT(fileType != AssetType::Default);
        AssetID id;
        switch (fileType)
        {
        case AssetType::Mesh:
            return defaultMesh;
            break;
        case AssetType::Texture2D:
            return mDefaultTexture;
            break;
        case AssetType::Shader:
            return mDefaultShader;
            break;
        case AssetType::Material:
            EnsureDefaultMaterial();
            return mDefaultMaterial.IsValid() ? mDefaultMaterial.Get() : nullptr;
            break;
        default:
            ASSERT(false);
            break;
        }
```
...
```cpp
    LoadTask *ResourceManager::GetOrCreateALoadTask(uint64_t assetid, AssetType assetType)
    {
        AssetID id(assetid);
        if(mLoadTaskCache.count(id) > 0) return mLoadTaskCache[id];

        if(freeTaskList.size() > 0)
        {
            LoadTask* task = freeTaskList.back();
            task->Reset();
            mLoadTaskCache[id] = task;
            freeTaskList.pop_back();
            return task;
        }
```
...
```cpp
        temp.SetPath(defaultTexturePath);
        temp.SetAssetID(AssetIDGenerator::NewFromFile(defaultTexturePath));
        AssetRegistry::GetInstance()->RegisterAsset(&temp);

        mDefaultTexture = static_cast<Texture*>(m_Loaders[AssetType::Texture2D]->Load(defaultTexturePath).resource);
        mDefaultTexture->OnLoadComplete();
        mResourceCache[temp.GetAssetID()] = static_cast<Resource*>(mDefaultTexture);
    }

    void ResourceManager::EnsureDefaultShader()
    {
```

### File: `Runtime/Serialization/MeshLoader.h`
```cpp
namespace EngineCore
{
    class MeshLoader : public IResourceLoader
    {
    public:
        virtual ~MeshLoader() = default;
        virtual LoadResult Load(const std::string& relativePath) override
        {
            LoadResult result;
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream in(path, std::ios::binary);
            in.seekg(sizeof(AssetHeader));

            Mesh* mesh = new Mesh();
            mesh->SetAssetCreateMethod(AssetCreateMethod::Serialization);
            mesh->SetAssetID(AssetIDGenerator::NewFromFile(path));
            StreamHelper::Read(in, mesh->bounds);
            StreamHelper::ReadVector(in, mesh->vertex);
            StreamHelper::ReadVector(in, mesh->index);
            result.resource = mesh; 
            return result;
        }

        void SaveMeshToBin(const Mesh* mesh, const std::string& relativePath, uint64_t id)
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
*/

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
```
...
```cpp
    };

    struct DDS_HEADER_DXT10 {
        uint32_t dxgiFormat;      // DXGI_FORMAT枚举值
        uint32_t resourceDimension; // D3D11_RESOURCE_DIMENSION
        uint32_t miscFlag;        // D3D11_RESOURCE_MISC_FLAG
        uint32_t arraySize;       // 数组大小
        uint32_t miscFlags2;      // 额外标志
    };
```
...
```cpp
    };

    class DDSTextureLoader : public IResourceLoader
    {
    public:
        virtual ~DDSTextureLoader() = default;
        virtual LoadResult Load(const std::string& relativePath) override
        {
            LoadResult result;
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

            result.resource = tex;
            return result;
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
        }
    
    private:
    
    
        DDSLoadResult LoadDDSFromFile(const std::string& relativePath)
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
```
...
```cpp
            if(header.magic != 0x20534444 || header.fileSize != 124)
            {
                ASSERT(false);
            }
    
            DDSLoadResult result;
            result.width = header.width;
            result.height = header.height;
            result.mipMapCount = header.mipMapCount > 0 ? header.mipMapCount : 1;
        
            // 7. 判断压缩格式 (DXT1/DXT3/DXT5)
            uint32_t fourCC = header.fourCC;
            
            // 提取fourCC字符串来判断
            char fourCCStr[5] = {0};
```
...
```cpp
                result.blockSize = 8;
            }
            else if (std::memcmp(fourCCStr, "DXT3", 4) == 0) {
                result.format = TextureFormat::DXT3;  // DXT3
                result.blockSize = 16;
            }
```

### File: `Runtime/Resources/ResourceManager.h`
```cpp
    };

    class ResourceManager
    {
    public:
        
        static void Create();
        void Destroy();
        ~ResourceManager();
        ResourceManager();

        void Update();
        void WorkThreadLoad();

        template<typename T>
        ResourceHandle<T> LoadAssetAsync(uint64_t assetPathID, std::function<void()> callback, LoadTask* parentTask)
        {
            LoadTask* currTask = GetOrCreateALoadTask(assetPathID, AssetTypeTraits<T>::Type);
            // LoadAssetAsync只负责发起异步加载任务，不关心任何比如资源加载完怎么处理
            if(currTask->loadState == LoadState::Finalized)
            {
                if(callback) callback();
                // 不用TryFinalize Parent，TryFinalize已经在父节点LoadResult的时候调用过了。
                return ResourceHandle<T>(assetPathID);
            }

            if(callback != nullptr)
            {
                currTask->calllbacks.push_back(callback);
            }
            if(parentTask != nullptr)
            {
                parentTask->pendingDeps++;
                currTask->calllbacks.push_back([=]()
                    {
                        parentTask->pendingDeps--;
                        TryFinalize(parentTask);
                    });
            }

            if(currTask->loadState == LoadState::None)
            {
                currTask->loadState = LoadState::Queue;
                currTask->id = assetPathID;
                currTask->type = AssetTypeTraits<T>::Type;
                mLoadTaskQueue.TryPush(currTask);
                mResourceCache[assetPathID] = GetDefaultResource(currTask->type);
            }

            return  ResourceHandle<T>(assetPathID);
        }

        ResourceState GetResourceStateByID(uint64_t assetID )
        {
            if(mResourceCache.count(assetID) == 0) return ResourceState::NotExits;
            if(mResourceCache[assetID] == nullptr) return ResourceState::Loading;
            if(mResourceCache[assetID] != nullptr) return ResourceState::Success;
        }

        // todo 异步加载和同步加载可能会冲突
        // 感觉mResourceCache赋值和取值的一瞬间应该加锁？
        // 这个地方应该是有问题， task = 不是none 的状态， 这个地方会和Async冲突
        template<typename T>
        ResourceHandle<T> LoadAsset(const string& relativePath)
        {
            uint64_t assetPathID = AssetRegistry::GetInstance()->GetAssetIDFromPath(relativePath);
            LoadTask* currTask = GetOrCreateALoadTask(assetPathID, AssetTypeTraits<T>::Type);
            // LoadAssetAsync只负责发起异步加载任务，不关心任何比如资源加载完怎么处理

            if (currTask->loadState == LoadState::None)
            {
                AssetType type = AssetTypeTraits<T>::Type;
                ASSERT(m_Loaders.count(type) > 0);
                string path = AssetRegistry::GetInstance()->GetAssetPathFromID(assetPathID);
                LoadResult result = m_Loaders[type]->Load(path);
                mResourceCache[assetPathID] = result.resource;
                mLoadTaskCache[assetPathID] = currTask;
                currTask->loadState = LoadState::Loaded;
                currTask->resource = result.resource;
                for(auto& dependency : result.dependencyList)
```
...
```cpp
                    {
                    case AssetType::Texture2D:
                        LoadAsset<Texture>(dependencyPath);
                        break;
                    case AssetType::Shader:
                        LoadAsset<Shader>(dependencyPath);
                        break;
                    default:
                        ASSERT(false);
                        break;
                    }

                    if(dependency.onLoaded)
                    {
```
...
```cpp
                TryFinalize(currTask);
            }
            return  ResourceHandle<T>(assetPathID);
        }


        template<typename T>
        ResourceHandle<T> Instantiate(const ResourceHandle<T>& sourceHandle)
        {
```
...
```cpp
        LoadTask* GetOrCreateALoadTask(uint64_t assetid, AssetType assetType);
        void TryFinalize(LoadTask* task);
        void EnsureDefaultTexture();
        void EnsureDefaultShader();
        void EnsureDefaultMaterial();

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
        std::string GetAssetPathFromID(uint64_t id);
        uint64_t GetAssetIDFromPath(const std::string& path);
        void SaveToDisk(const std::string& manifestPath);
        void LoadFromDisk(const std::string& manifestPath);
    private:
        static AssetRegistry* s_Instacnce;
        std::unordered_map<uint64_t, std::string> assetIDToPathMap;
        std::unordered_map<std::string, uint64_t> pathToAssetIDMap;

        std::mutex  m_mutex;;
    };
```

### File: `Runtime/Serialization/SceneLoader.h`
```cpp
    };

    class SceneLoader: public IResourceLoader                       
    {
    public:
        virtual ~SceneLoader() = default;

        // 场景序列化一般在主线程做， 确保数据安全。
        virtual LoadResult Load(const std::string& relativePath) override
        {
            LoadResult result;
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream in(path, std::ios::binary);

            // todo: 确保进来的文件是Scene类型
            in.seekg(sizeof(AssetHeader));

            Scene* scene = new Scene();
            SceneManager::GetInstance()->SetCurrentScene(scene);
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
                        }, nullptr);
                    
                    // 加载并设置 Material
                    if(nodeData.materialID != 0)
                    {
                        MeshRenderer* renderer = go->AddComponent<MeshRenderer>();
                        string path = AssetRegistry::GetInstance()->GetAssetPathFromID(nodeData.materialID);
                        // --- 添加这块临时调试代码 ---
                        if (path == "Material/bistro/Material_182.mat") {
                            int debug_here = 0; // 在这行打一个普通断点（F9）
                        }
                        ResourceHandle<Material> handle = ResourceManager::GetInstance()->LoadAsset<Material>(
                            path
                            );
                        renderer->OnLoadResourceFinished();
                        renderer->SetSharedMaterial(handle);

                    }
                }

            }
            result.resource = scene;
            return result;
        }

        void SaveSceneToBin(const Scene* scene, const std::string& relativePath, uint32_t id)
        {
            ASSERT(scene && scene->allObjList.size() > 0);
            std::string binPath = PathSettings::ResolveAssetPath(relativePath);
            std::ofstream out(binPath, std::ios::binary);

            AssetHeader header;
            header.assetID =id;
```
...
```cpp
            }
            
            StreamHelper::WriteVector(out, linearNode);
        }
    private:
        void SerilizedNode(GameObject* gameObject, std::unordered_map<GameObject*, uint32_t>& gameObjectMap, std::vector<SceneSerializedNode>& linearNode)
        {
```

### File: `Runtime/Resources/AssetTypeTraits.h`
```cpp
namespace EngineCore
{
    class Mesh;
    class Texture;
    class Material;
    class Shader;

    template<typename T> struct AssetTypeTraits { static const AssetType Type = AssetType::Default; };
```

### File: `Runtime/MaterialLibrary/MaterialLayout.h`
```cpp
{
    // 用来存
    struct TextureAssetInfo
    {
        // 这个地方直接记录
        char name[64];
        uint64_t ASSETID;
    };
```
...
```cpp
    };

    class MaterialLayout
    {
    public:

        void AddProp(const std::string& name, ShaderVariableType type, uint32_t size)
        {
            MaterialPropertyLayout prop;
            prop.name = name;
            prop.type = type;
            prop.size = size;
            prop.offset = m_TotalSize;
            m_PropertyLayout[name] = prop;
            
            m_TotalSize += size;
        }

        void AddTextureToBlockOffset(const std::string& name, uint32_t offset)
        {
            ASSERT(textureToBlockIndexMap.count(name) < 0);
            textureToBlockIndexMap[name] = offset;
            
        }

        uint32_t GetPropertyOffset(const std::string& name)
        {
            ASSERT(m_PropertyLayout.count(name) > 0);
            return m_PropertyLayout[name].offset;
        };

        uint32_t GetSize(){ return m_TotalSize;}

        std::unordered_map<std::string, MaterialPropertyLayout> m_PropertyLayout;
        std::unordered_map<std::string, uint32_t> textureToBlockIndexMap;
    private:
        uint32_t m_TotalSize = 0;
    };
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
    class Node;
    class Model;
    class Mesh;
}

namespace EngineCore {
```
...
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

### File: `Runtime/Serialization/ShaderLoader.h`
```cpp
namespace EngineCore
{
    class ShaderLoader : public IResourceLoader
    {
    public:
        virtual ~ShaderLoader() = default;
        virtual LoadResult Load(const std::string& relativePath) override
        {
            LoadResult result;
            std::string path = PathSettings::ResolveAssetPath(relativePath);

            Shader* shader = new Shader(path);
            result.resource = shader; 
            return result;
        }

    };
```

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