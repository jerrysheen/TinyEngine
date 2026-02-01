# Architecture Digest: SERIALIZATION
> Auto-generated. Focus: Runtime/Serialization, AssetSerialization, BaseTypeSerialization, ComponentFactory, JsonSerializer, MetaData, MetaFactory, MetaLoader, AssetHeader, StreamHelper, MeshRenderer, MeshFilter, Material, ModelData, MeshLoader, SceneLoader, MaterialLoader, ShaderLoader, DDSTextureLoader, IResourceLoader, ResourceManager, ResourceHandle

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
- `[72]` **Runtime/Serialization/MaterialLoader.h** *(Content Included)*
- `[61]` **Runtime/Serialization/SceneLoader.h** *(Content Included)*
- `[57]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[44]` **Runtime/Serialization/MeshLoader.h** *(Content Included)*
- `[41]` **Runtime/Resources/ResourceManager.cpp** *(Content Included)*
- `[39]` **Runtime/Serialization/DDSTextureLoader.h** *(Content Included)*
- `[37]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[37]` **Runtime/Resources/ResourceManager.h** *(Content Included)*
- `[35]` **Runtime/Serialization/ShaderLoader.h** *(Content Included)*
- `[34]` **Runtime/GameObject/MeshRenderer.cpp** *(Content Included)*
- `[34]` **Runtime/Scene/BistroSceneLoader.h** *(Content Included)*
- `[34]` **Runtime/Serialization/StreamHelper.h** *(Content Included)*
- `[33]` **Runtime/Serialization/AssetHeader.h** *(Content Included)*
- `[32]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[32]` **Runtime/Resources/ResourceHandle.h** *(Content Included)*
- `[29]` **Runtime/GameObject/MeshFilter.h** *(Content Included)*
- `[28]` **Runtime/Graphics/Material.cpp** *(Content Included)*
- `[27]` **Runtime/GameObject/MeshFilter.cpp** *(Content Included)*
- `[27]` **Runtime/MaterialLibrary/MaterialArchetypeRegistry.h** *(Content Included)*
- `[27]` **Runtime/MaterialLibrary/MaterialInstance.h** *(Content Included)*
- `[26]` **Runtime/MaterialLibrary/MaterialLayout.h**
- `[24]` **Runtime/Graphics/GPUSceneManager.cpp**
- `[24]` **Runtime/Resources/IResourceLoader.h**
- `[20]` **Runtime/Entry.cpp**
- `[19]` **Editor/Panel/EditorMainBar.cpp**
- `[17]` **Runtime/Core/Game.cpp**
- `[17]` **Runtime/MaterialLibrary/StandardPBR.h**
- `[17]` **Runtime/Scene/SceneManager.cpp**
- `[16]` **Runtime/Renderer/BatchManager.h**
- `[16]` **Runtime/Scene/Scene.h**
- `[15]` **Runtime/Scene/Scene.cpp**
- `[14]` **Runtime/MaterialLibrary/StandardPBR.cpp**
- `[12]` **Runtime/Core/PublicStruct.h**
- `[12]` **Runtime/Renderer/BatchManager.cpp**
- `[12]` **Runtime/Renderer/RenderContext.cpp**
- `[12]` **Runtime/Renderer/RenderEngine.cpp**
- `[12]` **Runtime/Renderer/Renderer.h**
- `[12]` **Runtime/Serialization/TextureLoader.h**
- `[12]` **Editor/Panel/EditorMainBar.h**
- `[10]` **Runtime/Scene/SceneManager.h**
- `[9]` **Runtime/Graphics/GPUSceneManager.h**
- `[9]` **Runtime/Renderer/Culling.cpp**
- `[7]` **Runtime/Renderer/RenderAPI.h**
- `[7]` **Runtime/Renderer/RenderCommand.h**
- `[7]` **Runtime/Renderer/RenderStruct.h**
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
            header.assetID =id;
            header.type = 3;
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

### File: `Runtime/Scene/BistroSceneLoader.cpp`
```cpp
    }

    void BistroSceneLoader::SaveToCache(Scene* scene, const std::string& path) {
        std::ofstream out(path, std::ios::binary);
        if (!out) {
            std::cout << "Failed to open cache file for writing: " << path << std::endl;
            return;
        }

        // Header
        const char* magic = "TINY";
        out.write(magic, 4);
        int version = 1;
        out.write(reinterpret_cast<const char*>(&version), sizeof(version));

        GameObject* root = nullptr;
        // Find BistroRoot - assumming it's one of the roots or we just take the first one?
        // BistroSceneLoader creates "BistroRoot"
        for (auto go : scene->rootObjList) {
            if (go->name == "BistroRoot") {
                root = go;
                break;
            }
        }
        
        if (!root && !scene->rootObjList.empty()) {
             root = scene->rootObjList[0];
        }

        if (!root) {
            std::cout << "No root object found to save." << std::endl;
            return;
        }

        // 1. Collect Meshes
        std::vector<Mesh*> distinctMeshes;
        std::unordered_map<Mesh*, int> meshMap;
        CollectMeshes(root, distinctMeshes, meshMap);

        // 2. Save Meshes
        size_t meshCount = distinctMeshes.size();
        out.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));

        for (Mesh* mesh : distinctMeshes) {
            // Bounds
            out.write(reinterpret_cast<const char*>(&mesh->bounds), sizeof(AABB));
            
            // Vertices
            size_t vCount = mesh->vertex.size();
            out.write(reinterpret_cast<const char*>(&vCount), sizeof(vCount));
            if (vCount > 0) {
                out.write(reinterpret_cast<const char*>(mesh->vertex.data()), vCount * sizeof(Vertex));
            }

            // Indices
            size_t iCount = mesh->index.size();
            out.write(reinterpret_cast<const char*>(&iCount), sizeof(iCount));
            if (iCount > 0) {
                out.write(reinterpret_cast<const char*>(mesh->index.data()), iCount * sizeof(int));
            }

            // Layout
            size_t lCount = mesh->layout.size();
            out.write(reinterpret_cast<const char*>(&lCount), sizeof(lCount));
            if (lCount > 0) {
                out.write(reinterpret_cast<const char*>(mesh->layout.data()), lCount * sizeof(InputLayout));
            }
        }

        // 3. Save Node Hierarchy
        SaveNode(root, out, meshMap);

        out.close();
        std::cout << "Saved scene to cache: " << path << std::endl;
    }

    GameObject* LoadNode(Scene* scene, GameObject* parent, std::ifstream& in, const std::vector<ResourceHandle<Mesh>>& meshes) {
        // Name
        std::string name = ReadString(in);
        GameObject* go = scene->CreateGameObject(name);
```
...
```cpp

        ProcessTexture(model);
        ProcessMaterials(model);


        for (size_t i = 0; i < scene.nodes.size(); i++) {
            ProcessNode(model.nodes[scene.nodes[i]], model, rootGo, newScene);
        }
```
...
```cpp
    }

    void BistroSceneLoader::ProcessMesh(int meshIndex, const tinygltf::Model& model, GameObject* go, Scene* targetScene) {
        std::vector<std::pair<ResourceHandle<EngineCore::Mesh>, int>> modelHandles;

        // Check Cache
        auto it = m_MeshCache.find(meshIndex);
        if (it != m_MeshCache.end()) {
            modelHandles = it->second;
        } else {
            const tinygltf::Mesh& gltfMesh = model.meshes[meshIndex];

            // Iterate all Primitives
            for (const auto& primitive : gltfMesh.primitives) {
                ResourceHandle<EngineCore::Mesh> modelHandle = ResourceManager::GetInstance()->CreateResource<EngineCore::Mesh>();
                EngineCore::Mesh* meshRes = modelHandle.Get();
                meshRes->bounds = AABB();

                // 1. Get Accessors and Buffers
                // Indices
                if (primitive.indices > -1) {
                    const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                    const tinygltf::BufferView& bufferView = model.bufferViews[indexAccessor.bufferView];
                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                    const unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + indexAccessor.byteOffset;
                    size_t count = indexAccessor.count;
                    int componentType = indexAccessor.componentType; 

                    meshRes->index.reserve(count);
                    if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        const unsigned short* buf = reinterpret_cast<const unsigned short*>(dataStart);
                        for (size_t i = 0; i < count; i++) meshRes->index.push_back(buf[i]);
                    } else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                        const unsigned int* buf = reinterpret_cast<const unsigned int*>(dataStart);
                        for (size_t i = 0; i < count; i++) meshRes->index.push_back(buf[i]);
                    }
                }

                // Position
                if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                    
                    const unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
                    size_t count = accessor.count;
                    int stride = accessor.ByteStride(bufferView); 

                    meshRes->vertex.resize(count);
                    for (size_t i = 0; i < count; i++) {
                        const float* buf = reinterpret_cast<const float*>(dataStart + i * stride);
                        meshRes->vertex[i].position = Vector3(buf[0], buf[1], buf[2]);
                        meshRes->bounds.Encapsulate(meshRes->vertex[i].position);
                    }
                }

                // Normal
                if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                    
                    const unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
                    size_t count = accessor.count;
                    int stride = accessor.ByteStride(bufferView);

                    for (size_t i = 0; i < count; i++) {
                        const float* buf = reinterpret_cast<const float*>(dataStart + i * stride);
                        meshRes->vertex[i].normal = Vector3(buf[0], buf[1], buf[2]);
                    }
                }

                // UV0
                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                    
                    const unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
```
...
```cpp
                     meshRes->SetAssetID(newID);

                     ResourceManager::GetInstance()->mResourceCache.erase(runtimeID);
                     ResourceManager::GetInstance()->mResourceCache[newID] = meshRes;

                     modelHandle = ResourceHandle<EngineCore::Mesh>(newID);

                     MeshLoader loader;
                     loader.SaveMeshToBin(meshRes, assetPath, (uint32_t)newID);

                     AssetRegistry::GetInstance()->RegisterAsset(meshRes);
                 }

                modelHandles.push_back({modelHandle, primitive.material});
```
...
```cpp

            // 添加 MeshRenderer 并绑定 Material
            AttachMaterialToGameObject(obj, materialIndex);
        };

        // Create GameObject components
        if (modelHandles.size() == 1) {
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
            //SetUpMaterialPropertyBlock();
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
        void OnLoadResourceFinished();

    private:
        ResourceHandle<Material> mShardMatHandler;
        ResourceHandle<Material> mInstanceMatHandler;

    };
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
            AssetID id = resource->GetAssetID();
            mResourceCache[id] = resource;
            return ResourceHandle<T>(id);
        }

        inline void AddRef(AssetID id)
        {
```
...
```cpp
            AssetID id = resource->GetAssetID();
            mResourceCache[id] = resource;
            return ResourceHandle<T>(id);
        }

        Resource* GetDefaultResource(AssetType fileType);

    // todo: temp public for Editor Test
    public:
        static ResourceManager* sInstance;

        std::vector<Resource*> mPendingDeleteList;
        std::unordered_map<AssetType, IResourceLoader*> m_Loaders;
        

        std::thread mLoadThread;
        Mesh* defaultMesh = nullptr;
        Texture* mDefaultTexture = nullptr;
        Shader* mDefaultShader = nullptr;
        ResourceHandle<Material> mDefaultMaterial;
        
        std::vector<LoadTask*> freeTaskList;
        unordered_map<AssetID, Resource*> mResourceCache;
        unordered_map<AssetID, LoadTask*> mLoadTaskCache;
        // LoadTask 不回收
        ThreadSafeQueue<LoadTask*> mLoadTaskQueue;
        ThreadSafeQueue<LoadResult> mLoadResultQueue;
        LoadTask* GetOrCreateALoadTask(uint64_t assetid, AssetType assetType);
        void TryFinalize(LoadTask* task);
        void EnsureDefaultTexture();
        void EnsureDefaultShader();
        void EnsureDefaultMaterial();

    };

}
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

### File: `Runtime/Resources/ResourceHandle.h`
```cpp
    // 但是Resource的拷贝，移动等，会涉及到Ref++；
    template<typename T>
    class ResourceHandle
    {
    public:
        ResourceHandle() = default;

        ResourceHandle(AssetID id);
        
        // 拷贝构造：
        ResourceHandle(const ResourceHandle& other);

        ResourceHandle(ResourceHandle&& other) noexcept : mAssetID(other.mAssetID)
        {
            // 原来的要 releaseRef， 新的要AddRef所以刚刚好抵消。
            // 当前内容已经在初始化列表中赋值了
            other.mAssetID.Reset();
        }

        ~ResourceHandle();

        // 赋值拷贝
        ResourceHandle& operator=(const ResourceHandle& other);

        //移动赋值：
        ResourceHandle& operator=(ResourceHandle&& other) noexcept;

        // ReourceHandle.Get()， 获得具体的Resource，
        T* Get() const;

        T* operator ->() const 
        {
            return Get();
        }

        // 解引用
        T& operator*() const {
            return *Get();
        }

        // 判断是否有效
        bool IsValid() const {
            return mAssetID.IsValid() && Get() != nullptr;
        }

        ResourceState GetResourecState()
        {
            return ResourceManager::GetInstance()->GetResourceStateByID(mAssetID.value);
        }

        inline AssetID GetAssetID() const {return mAssetID;};
    // todo : temp data for jason sereilization;
    public :
        AssetID mAssetID;
    };
}
#include "ResourceManager.h"
namespace EngineCore
{
    template<typename T>
    inline ResourceHandle<T>::ResourceHandle(AssetID id)
        : mAssetID(id)
    {
        if (mAssetID.IsValid())
        {
            ResourceManager::GetInstance()->AddRef(mAssetID);
        }
    }

    template <typename T>
    inline ResourceHandle<T>::ResourceHandle(const ResourceHandle &other)
        : mAssetID(other.GetAssetID())
    {
        if(mAssetID.IsValid())
        {
            ResourceManager::GetInstance()->AddRef(mAssetID);
        }
    }
    
    template <typename T>
```
...
```cpp
        if(mAssetID.IsValid())
        {
            ResourceManager::GetInstance()->DecreaseRef(mAssetID);
        }
    }

    
    template <typename T>
    inline ResourceHandle<T>& ResourceHandle<T>::operator=(const ResourceHandle& other)
    {
```
...
```cpp
            if(mAssetID.IsValid())
            {
                ResourceManager::GetInstance()->AddRef(mAssetID);
            }

            //  释放现在的
           if(oldID.IsValid())
           {
```
...
```cpp
                if(mAssetID.IsValid())
                {
                    ResourceManager::GetInstance()->AddRef(mAssetID);
                }
                // 转移所有权
                mAssetID = other.mAssetID;
                other.mAssetID.Reset();  // 清空 other
            }
            // 如果两者一样的话，就不做操作， 将亡值走自己的析构函数 decrease ref
            return *this;

    }

    template <typename T>
    inline T *ResourceHandle<T>::Get() const
    {
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