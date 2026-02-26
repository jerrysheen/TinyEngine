#include "PreCompiledHeader.h"
#include "BistroSceneLoader.h"
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include "External/tinygltf/tiny_gltf.h"

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"
#include "Graphics/Mesh.h"
#include "GameObject/GameObject.h"

#include "Renderer/RenderAPI.h"
#include "Scene.h"
#include "Settings/ProjectSettings.h"
#include <fstream>
#include <unordered_map>
#include "Serialization/MeshLoader.h"
#include "Serialization/SceneLoader.h"
#include "Serialization/MaterialLoader.h"
#include "Serialization/DDSTextureLoader.h"
#include "Resources/AssetRegistry.h"
#include "Resources/Asset.h"
#include <cstring>
#include "MaterialLibrary/MaterialArchetypeRegistry.h"


namespace EngineCore {
    ResourceHandle<Material> BistroSceneLoader::commonMatHandle;

    Scene* BistroSceneLoader::Load(const std::string& path) {
        AssetRegistry::GetInstance()->LoadFromDisk("AssetRegistry.bin");

        EngineCore::SceneLoader sceneLoader;
        std::string binPath = "Scenes/BistroScene.bin";
        std::string fullPath = PathSettings::ResolveAssetPath(binPath);
        
        std::ifstream cacheFile(fullPath, std::ios::binary);
        if (cacheFile.good()) {
            cacheFile.close();
            std::cout << "Loading from cache: " << fullPath << std::endl;
            Scene* res = static_cast<Scene*>(sceneLoader.Load(binPath).resource);
            // todo： 这个地方有加载时序问题。。
            SceneManager::GetInstance()->SetCurrentScene(res);
            // 刷新所有Transform的world position，避免父子节点关系建立后的延迟更新问题
            for (auto& gameObject : res->rootObjList) 
            {
                if (gameObject != nullptr && gameObject->transform != nullptr)
                {
                    gameObject->transform->UpdateRecursively(0);
                }
            }
        
            return res;
        }

        BistroSceneLoader loader;
        return loader.LoadInternal(path);
    }

    // Helper functions for binary IO
    void WriteString(std::ofstream& out, const std::string& str) {
        size_t len = str.length();
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        if (len > 0) out.write(str.c_str(), len);
    }

    std::string ReadString(std::ifstream& in) {
        size_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (len > 0) {
            std::string str(len, '\0');
            in.read(&str[0], len);
            return str;
        }
        return "";
    }

    void WriteVector3(std::ofstream& out, const Vector3& v) {
        out.write(reinterpret_cast<const char*>(&v), sizeof(Vector3));
    }

    Vector3 ReadVector3(std::ifstream& in) {
        Vector3 v;
        in.read(reinterpret_cast<char*>(&v), sizeof(Vector3));
        return v;
    }

    void WriteQuaternion(std::ofstream& out, const Quaternion& q) {
        out.write(reinterpret_cast<const char*>(&q), sizeof(Quaternion));
    }

    Quaternion ReadQuaternion(std::ifstream& in) {
        Quaternion q;
        in.read(reinterpret_cast<char*>(&q), sizeof(Quaternion));
        return q;
    }

    void CollectMeshes(GameObject* node, std::vector<Mesh*>& distinctMeshes, std::unordered_map<Mesh*, int>& meshMap) {
        MeshFilter* mf = node->GetComponent<MeshFilter>();
        if (mf && mf->mMeshHandle.IsValid()) {
            Mesh* mesh = mf->mMeshHandle.Get();
            if (meshMap.find(mesh) == meshMap.end()) {
                meshMap[mesh] = (int)distinctMeshes.size();
                distinctMeshes.push_back(mesh);
            }
        }

        for (auto child : node->GetChildren()) {
            CollectMeshes(child, distinctMeshes, meshMap);
        }
    }

    void SaveNode(GameObject* node, std::ofstream& out, std::unordered_map<Mesh*, int>& meshMap) {
        // Name
        WriteString(out, node->name);
        
        // Transform
        WriteVector3(out, node->transform->GetLocalPosition());
        WriteQuaternion(out, node->transform->GetLocalQuaternion());
        WriteVector3(out, node->transform->GetLocalScale());

        // Mesh
        int meshID = -1;
        MeshFilter* mf = node->GetComponent<MeshFilter>();
        if (mf && mf->mMeshHandle.IsValid()) {
            Mesh* mesh = mf->mMeshHandle.Get();
            if (meshMap.count(mesh)) {
                meshID = meshMap[mesh];
            }
        }
        out.write(reinterpret_cast<const char*>(&meshID), sizeof(meshID));

        // Children
        const auto& children = node->GetChildren();
        size_t childCount = children.size();
        out.write(reinterpret_cast<const char*>(&childCount), sizeof(childCount));

        for (auto child : children) {
            SaveNode(child, out, meshMap);
        }
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
        int version = 2;
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
        go->SetParent(parent);

        // Transform
        Vector3 pos = ReadVector3(in);
        Quaternion rot = ReadQuaternion(in);
        Vector3 scale = ReadVector3(in);

        go->transform->SetLocalPosition(pos);
        go->transform->SetLocalQuaternion(rot);
        go->transform->SetLocalScale(scale);

        // Mesh
        int meshID;
        in.read(reinterpret_cast<char*>(&meshID), sizeof(meshID));
        if (meshID >= 0 && meshID < meshes.size()) {
            MeshFilter* mf = go->AddComponent<MeshFilter>();
            mf->mMeshHandle = meshes[meshID];

            MeshRenderer* mr = go->AddComponent<MeshRenderer>();
            
            // Re-use common material logic
             std::string shaderName = "Shader/StandardPBR.hlsl";
            if(RenderSettings::s_EnableVertexPulling)
            {
                shaderName = "Shader/StandardPBR_VertexPulling.hlsl";
            }
            ResourceHandle<Shader> pbrShader = ResourceManager::GetInstance()->LoadAsset<Shader>(shaderName);
            
            if (pbrShader.IsValid()) {
                if (!BistroSceneLoader::commonMatHandle.IsValid()) 
                {
                    BistroSceneLoader::commonMatHandle = ResourceManager::GetInstance()->CreateResource<Material>(pbrShader);
                }

                mr->SetSharedMaterial(BistroSceneLoader::commonMatHandle);
            } 
        }

        // Children
        size_t childCount;
        in.read(reinterpret_cast<char*>(&childCount), sizeof(childCount));

        for (size_t i = 0; i < childCount; i++) {
            LoadNode(scene, go, in, meshes);
        }

        return go;
    }

    Scene* BistroSceneLoader::LoadFromCache(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in) return nullptr;

        char magic[5] = {0};
        in.read(magic, 4);
        if (std::string(magic) != "TINY") {
            std::cout << "Invalid cache file magic" << std::endl;
            return nullptr;
        }

        int version;
        in.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != 2) {
             std::cout << "Invalid cache file version" << std::endl;
             return nullptr;
        }

        Scene* newScene = new Scene("BistroScene");
        SceneManager::GetInstance()->SetCurrentScene(newScene);

        // 1. Load Meshes
        size_t meshCount;
        in.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
        std::vector<ResourceHandle<Mesh>> loadedMeshes;
        loadedMeshes.reserve(meshCount);

        for (size_t i = 0; i < meshCount; i++) {
            ResourceHandle<Mesh> handle = ResourceManager::GetInstance()->CreateResource<Mesh>();
            Mesh* mesh = handle.Get();

            // Bounds
            in.read(reinterpret_cast<char*>(&mesh->bounds), sizeof(AABB));

            // Vertices
            size_t vCount;
            in.read(reinterpret_cast<char*>(&vCount), sizeof(vCount));
            if (vCount > 0) {
                mesh->vertex.resize(vCount);
                in.read(reinterpret_cast<char*>(mesh->vertex.data()), vCount * sizeof(Vertex));
            }

            // Indices
            size_t iCount;
            in.read(reinterpret_cast<char*>(&iCount), sizeof(iCount));
            if (iCount > 0) {
                mesh->index.resize(iCount);
                in.read(reinterpret_cast<char*>(mesh->index.data()), iCount * sizeof(int));
            }

            // Layout
            size_t lCount;
            in.read(reinterpret_cast<char*>(&lCount), sizeof(lCount));
            if (lCount > 0) {
                mesh->layout.resize(lCount);
                in.read(reinterpret_cast<char*>(mesh->layout.data()), lCount * sizeof(InputLayout));
            }

            mesh->UploadMeshToGPU();
            loadedMeshes.push_back(handle);
        }

        // 2. Load Hierarchy
        LoadNode(newScene, nullptr, in, loadedMeshes); // Root has no parent

        return newScene;
    }

    Scene* BistroSceneLoader::LoadInternal(const std::string& path) {
        ProcessShaders();
        CreateDefaultResources();

        tinygltf::Model model;
        tinygltf::TinyGLTF loader;

        // Set dummy image loader to prevent tinygltf from loading/decoding images immediately
        // We will handle texture loading separately using the Texture class
        loader.SetImageLoader([](tinygltf::Image* image, const int image_idx, std::string* err,
            std::string* warn, int req_width, int req_height,
            const unsigned char* bytes, int size, void* user_data) {
            return true;
        }, nullptr);

        std::string err;
        std::string warn;

        // 1. Load Definition & Data
        // Note: This loads the entire glTF structure including buffers into memory.
        // For a 100MB file, 150MB memory usage is expected due to internal structure overhead.
        // This 'model' variable is local to this function and will be destructed (releasing all temporary memory)
        // once the function returns, leaving only the converted ModelData in the ResourceManager.
        bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

        if (!warn.empty()) {
            std::cout << "TinyGLTF Warning: " << warn << std::endl;
        }

        if (!err.empty()) {
            std::cout << "TinyGLTF Error: " << err << std::endl;
        }

        if (!ret) {
            std::cout << "Failed to parse glTF" << std::endl;
            return nullptr;
        }

        Scene* newScene = new Scene("BistroScene");
        SceneManager::GetInstance()->SetCurrentScene(newScene);
        // Create root node
        GameObject* rootGo = newScene->CreateGameObject("BistroRoot");

        // GLTF can have multiple Scenes, default to defaultScene
        const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
        
        // Clear cache before starting
        m_MeshCache.clear();
        m_ImageIndexToID.clear();
        m_MaterialMap.clear();

        ProcessTexture(model);
        ProcessMaterials(model);


        for (size_t i = 0; i < scene.nodes.size(); i++) {
            ProcessNode(model.nodes[scene.nodes[i]], model, rootGo, newScene);
        }

        // Save serialized scene
        EngineCore::SceneLoader sceneLoader;
        // Use a temporary ID for now, or fetch from registry if available
        AssetRegistry::GetInstance()->SaveToDisk("AssetRegistry.bin");
        sceneLoader.SaveSceneToBin(newScene, "/Scenes/BistroScene.bin", 0);

        // Cache is cleared when loader instance is destroyed (end of Load function)
        return newScene;
    }

    void BistroSceneLoader::ProcessNode(const tinygltf::Node& node, const tinygltf::Model& model, GameObject* parent, Scene* targetScene) 
    {
        GameObject* go = targetScene->CreateGameObject(node.name.empty() ? "Node" : node.name);
        go->SetParent(parent);

        // Transform
        if (node.translation.size() == 3) {
            go->transform->SetLocalPosition(Vector3((float)node.translation[0], (float)node.translation[1], (float)node.translation[2]));
        }
        if (node.rotation.size() == 4) {
            go->transform->SetLocalQuaternion(Quaternion((float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3]));
        }
        if (node.scale.size() == 3) {
            go->transform->SetLocalScale(Vector3((float)node.scale[0], (float)node.scale[1], (float)node.scale[2]));
        }
        
        // Handle Matrix if present (simplified assumption: T/R/S is preferred)

        // Process Mesh
        // node.mesh is the index into model.meshes
        if (node.mesh > -1) {
            ProcessMesh(node.mesh, model, go, targetScene);
        }

        // Recursive children
        for (size_t i = 0; i < node.children.size(); i++) {
            ProcessNode(model.nodes[node.children[i]], model, go, targetScene);
        }
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
                        meshRes->vertex[i].tangent = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
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
                    size_t count = accessor.count;
                    int stride = accessor.ByteStride(bufferView);

                    for (size_t i = 0; i < count; i++) {
                        const float* buf = reinterpret_cast<const float*>(dataStart + i * stride);
                        meshRes->vertex[i].uv = Vector2(buf[0], buf[1]);
                    }
                }

                // Tangent
                if (primitive.attributes.find("TANGENT") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TANGENT")->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                    const unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
                    size_t count = accessor.count;
                    int stride = accessor.ByteStride(bufferView);

                    for (size_t i = 0; i < count; i++) {
                        const float* buf = reinterpret_cast<const float*>(dataStart + i * stride);
                        meshRes->vertex[i].tangent = Vector4(buf[0], buf[1], buf[2], buf[3]);
                    }
                }

                // Set Layout
                const int vertexStride = sizeof(Vertex);
                meshRes->layout.push_back(InputLayout(VertexAttribute::POSITION, sizeof(Vector3), 3, vertexStride, 0));
                meshRes->layout.push_back(InputLayout(VertexAttribute::NORMAL, sizeof(Vector3), 3, vertexStride, sizeof(Vector3)));
                meshRes->layout.push_back(InputLayout(VertexAttribute::UV0, sizeof(Vector2), 2, vertexStride, sizeof(Vector3) * 2));
                meshRes->layout.push_back(InputLayout(VertexAttribute::TANGENT, sizeof(Vector4), 4, vertexStride, sizeof(Vector3) * 2 + sizeof(Vector2)));

                // Upload to GPU
                meshRes->UploadMeshToGPU();

                 {
                     std::string meshName = "Bistro_Mesh_" + std::to_string(meshIndex) + "_" + std::to_string(modelHandles.size());
                     std::string assetPath = "Mesh/" + meshName + ".bin";

                     AssetID runtimeID = meshRes->GetAssetID();
                     AssetID newID = AssetIDGenerator::NewFromFile(assetPath);

                     meshRes->SetPath(assetPath);
                     meshRes->SetAssetCreateMethod(AssetCreateMethod::Serialization);
                     meshRes->SetAssetID(newID);

                     ResourceManager::GetInstance()->mResourceCache.erase(runtimeID);
                     ResourceManager::GetInstance()->mResourceCache[newID] = meshRes;

                     modelHandle = ResourceHandle<EngineCore::Mesh>(newID);

                     MeshLoader loader;
                     loader.SaveMeshToBin(meshRes, assetPath, (uint32_t)newID);

                     AssetRegistry::GetInstance()->RegisterAsset(meshRes);
                 }

                modelHandles.push_back({modelHandle, primitive.material});
            }
            
            // Add to cache
            m_MeshCache[meshIndex] = modelHandles;
        }

        if (modelHandles.empty()) return;

        auto CreateMeshComponents = [&](GameObject* obj, ResourceHandle<EngineCore::Mesh> meshHandle, int materialIndex) {
            // 添加 MeshFilter 组件
            MeshFilter* mf = obj->AddComponent<MeshFilter>();
            mf->mMeshHandle = meshHandle;

            // 添加 MeshRenderer 并绑定 Material
            AttachMaterialToGameObject(obj, materialIndex);
        };

        // Create GameObject components
        if (modelHandles.size() == 1) {
            CreateMeshComponents(go, modelHandles[0].first, modelHandles[0].second);
        } else {
            // Multiple primitives become child objects
            for (size_t i = 0; i < modelHandles.size(); i++) {
                GameObject* subGo = targetScene->CreateGameObject(go->name + "_SubMesh_" + std::to_string(i));
                subGo->SetParent(go);
                CreateMeshComponents(subGo, modelHandles[i].first, modelHandles[i].second);
            }
        }
    }

    void BistroSceneLoader::AttachMaterialToGameObject(GameObject* gameObject, int materialIndex)
    {
        ASSERT(gameObject != nullptr);

        // 添加 MeshRenderer 组件
        MeshRenderer* mr = gameObject->AddComponent<MeshRenderer>();
        
        // 尝试从 MaterialMap 中获取对应的 Material
        bool materialAssigned = false;
        if (materialIndex >= 0 && materialIndex < m_MaterialMap.size()) {
            if (m_MaterialMap[materialIndex].IsValid()) {
                mr->SetSharedMaterial(m_MaterialMap[materialIndex]);
                materialAssigned = true;
            }
        }

        // 如果没有找到对应的 Material，使用默认的 PBR Material
        if (!materialAssigned) {
            ASSERT(false);
            std::string shaderName = "Shader/StandardPBR.hlsl";
            if (RenderSettings::s_EnableVertexPulling)
            {
                shaderName = "Shader/StandardPBR_VertexPulling.hlsl";
            }
            ResourceHandle<Shader> pbrShader = ResourceManager::GetInstance()->LoadAsset<Shader>(shaderName);
            
            if (pbrShader.IsValid()) {
                if (!commonMatHandle.IsValid()) 
                {
                    commonMatHandle = ResourceManager::GetInstance()->CreateResource<Material>(pbrShader);
                }

                mr->SetSharedMaterial(commonMatHandle);
            }
        }

    }

    void BistroSceneLoader::ProcessTexture(const tinygltf::Model &model)
    {
        m_ImageIndexToID.resize(model.images.size());

        for (size_t i = 0; i < model.images.size(); i++)
        {
            const tinygltf::Image& image = model.images[i];
            
            // 跳过没有 uri 的图像（可能是嵌入的图像）
            if (image.uri.empty()) continue;
            
            // 转换路径：将相对路径转换为 Assets/Textures/bistro/ 开头的路径
            std::string texturePath = "Textures/bistro/" + image.uri;
            
            // 创建一个临时的 Texture 对象用于注册
            Texture* tempTexture = new Texture();
            
            // 设置路径
            tempTexture->SetPath(texturePath);
            
            // 根据路径生成 AssetID
            tempTexture->SetAssetID(AssetIDGenerator::NewFromFile(texturePath));
            
            // 设置创建方法为序列化
            tempTexture->SetAssetCreateMethod(AssetCreateMethod::Serialization);
            
            // 注册到 AssetRegistry
            AssetRegistry::GetInstance()->RegisterAsset(tempTexture);

            m_ImageIndexToID[i] = tempTexture->GetAssetID();
            
            delete tempTexture;
        }
    }
    void BistroSceneLoader::ProcessMaterials(const tinygltf::Model& model)
    {
        m_MaterialMap.resize(model.materials.size());

        std::string shaderName = "Shader/StandardPBR.hlsl";
        if (RenderSettings::s_EnableVertexPulling)
        {
            shaderName = "Shader/StandardPBR_VertexPulling.hlsl";
        }
        ResourceHandle<Shader> pbrShader = ResourceManager::GetInstance()->LoadAsset<Shader>(shaderName);

        pbrShader->name = "StandardPBR";
        for (size_t i = 0; i < model.materials.size(); i++)
        {
            const tinygltf::Material& mat = model.materials[i];
            
            // Create Material
            ResourceHandle<Material> matHandle = ResourceManager::GetInstance()->CreateResource<Material>(pbrShader);
            Material* material = matHandle.Get();

            // 1. PBR Factors (统一到 Spec-Gloss 语义)
            bool useSpecGloss = false;
            Vector4 baseColorFactor(1.0f, 1.0f, 1.0f, 1.0f);
            Vector3 specularFactor(1.0f, 1.0f, 1.0f);
            float glossinessFactor = 1.0f;
            float metallicFactor = 0.0f;
            float roughnessFactor = 1.0f;

            if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end())
            {
                useSpecGloss = true;
                const auto& ext = mat.extensions.at("KHR_materials_pbrSpecularGlossiness");
                if (ext.Has("diffuseFactor"))
                {
                    auto v = ext.Get("diffuseFactor");
                    if (v.IsArray() && v.ArrayLen() == 4)
                    {
                        baseColorFactor = Vector4((float)v.Get(0).Get<double>(),
                            (float)v.Get(1).Get<double>(),
                            (float)v.Get(2).Get<double>(),
                            (float)v.Get(3).Get<double>());
                    }
                }
                if (ext.Has("specularFactor"))
                {
                    auto v = ext.Get("specularFactor");
                    if (v.IsArray() && v.ArrayLen() == 3)
                    {
                        specularFactor = Vector3((float)v.Get(0).Get<double>(),
                            (float)v.Get(1).Get<double>(),
                            (float)v.Get(2).Get<double>());
                    }
                }
                if (ext.Has("glossinessFactor"))
                {
                    glossinessFactor = (float)ext.Get("glossinessFactor").Get<double>();
                }
            }
            else
            {
                // pbrMetallicRoughness -> Spec-Gloss 近似转换
                if (mat.values.find("baseColorFactor") != mat.values.end())
                {
                    auto f = mat.values.at("baseColorFactor").ColorFactor();
                    baseColorFactor = Vector4((float)f[0], (float)f[1], (float)f[2], (float)f[3]);
                }
                if (mat.values.find("metallicFactor") != mat.values.end())
                {
                    metallicFactor = (float)mat.values.at("metallicFactor").Factor();
                }
                if (mat.values.find("roughnessFactor") != mat.values.end())
                {
                    roughnessFactor = (float)mat.values.at("roughnessFactor").Factor();
                }
            }

            Vector4 diffuseColor = baseColorFactor;
            Vector4 specularColor(1.0f, 1.0f, 1.0f, 1.0f);
            float roughness = roughnessFactor;

            if (useSpecGloss)
            {
                specularColor = Vector4(specularFactor.x, specularFactor.y, specularFactor.z, 1.0f);
                roughness = 1.0f - glossinessFactor;
                metallicFactor = 0.0f;
            }
            else
            {
                float oneMinusMetallic = 1.0f - metallicFactor;
                diffuseColor = Vector4(baseColorFactor.x * oneMinusMetallic,
                    baseColorFactor.y * oneMinusMetallic,
                    baseColorFactor.z * oneMinusMetallic,
                    baseColorFactor.w);

                const float dielectricF0 = 0.04f;
                specularColor = Vector4(
                    dielectricF0 + (baseColorFactor.x - dielectricF0) * metallicFactor,
                    dielectricF0 + (baseColorFactor.y - dielectricF0) * metallicFactor,
                    dielectricF0 + (baseColorFactor.z - dielectricF0) * metallicFactor,
                    1.0f);
            }

            if (mat.extensions.find("KHR_materials_transmission") != mat.extensions.end())
            {
                const auto& transExt = mat.extensions.at("KHR_materials_transmission");
                if (transExt.Has("transmissionFactor"))
                {
                    float transmissionFactor = (float)transExt.Get("transmissionFactor").Get<double>();
                    diffuseColor.w = diffuseColor.w * (1.0f - transmissionFactor);
                }
            }

            material->SetValue("DiffuseColor", &diffuseColor, sizeof(Vector4));
            material->SetValue("SpecularColor", &specularColor, sizeof(Vector4));
            material->SetValue("Roughness", &roughness, sizeof(float));
            material->SetValue("Metallic", &metallicFactor, sizeof(float));

            // 2. Textures
            // Diffuse / BaseColor
            if (mat.values.find("baseColorTexture") != mat.values.end())
            {
                int texIndex = mat.values.at("baseColorTexture").TextureIndex();
                uint64_t diffuseTextureID = GetTextureAssetID(model, texIndex);
                material->SetTexture("DiffuseTexture", diffuseTextureID);
            }

            // KHR_materials_pbrSpecularGlossiness
            if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end())
            {
                const auto& ext = mat.extensions.at("KHR_materials_pbrSpecularGlossiness");
                if (ext.Has("diffuseTexture"))
                {
                    int texIndex = ext.Get("diffuseTexture").Get("index").Get<int>();
                    uint64_t diffuseTextureID = GetTextureAssetID(model, texIndex);
                    material->SetTexture("DiffuseTexture", diffuseTextureID);
                }
                if (ext.Has("specularGlossinessTexture"))
                {
                    int texIndex = ext.Get("specularGlossinessTexture").Get("index").Get<int>();
                    uint64_t specGlossTexture = GetTextureAssetID(model, texIndex);
                    material->SetTexture("MetallicTexture", specGlossTexture);
                }
            }

            // Normal
            if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end())
            {
                int texIndex = mat.additionalValues.at("normalTexture").TextureIndex();
                uint64_t normalTextureIndex = GetTextureAssetID(model, texIndex);
                material->SetTexture("NormalTexture", normalTextureIndex);
            }

            // Emissive
            if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end())
            {
                int texIndex = mat.additionalValues.at("emissiveTexture").TextureIndex();
                uint64_t emissiveTextureIndex = GetTextureAssetID(model, texIndex);
                material->SetTexture("EmissiveTexture", emissiveTextureIndex);               
            }

            // 3. 设置 Material 的路径和 AssetID（参考 Mesh 的处理方式）
            {
                std::string matFileName = "Material/bistro/Material_" + std::to_string(i) + ".mat";
                std::string fullPath = PathSettings::ResolveAssetPath(matFileName);

                AssetID runtimeID = material->GetAssetID();
                AssetID newID = AssetIDGenerator::NewFromFile(matFileName);

                material->SetPath(matFileName);
                material->SetAssetCreateMethod(AssetCreateMethod::Serialization);
                material->SetAssetID(newID);
                //todo：  temp 先跑通
                material->archyTypeName = "StandardPBR";
                // 更新 ResourceManager 缓存
                ResourceManager::GetInstance()->mResourceCache.erase(runtimeID);
                ResourceManager::GetInstance()->mResourceCache[newID] = material;

                matHandle = ResourceHandle<Material>(newID);

                MaterialLoader loader;
                loader.SaveMaterialToBin(material, matFileName, newID);

                // 5. 注册到 AssetRegistry
                AssetRegistry::GetInstance()->RegisterAsset(material);
            }
            
            m_MaterialMap[i] = matHandle;
        }
    }

    void BistroSceneLoader::ProcessShaders()
    {
        std::string shaderName = "Shader/BlitShader.hlsl";
        Shader shader;
        shader.SetAssetCreateMethod(AssetCreateMethod::Serialization);
        shader.SetPath(shaderName);
        shader.SetAssetID(AssetIDGenerator::NewFromFile(shader.GetPath()));
        AssetRegistry::GetInstance()->RegisterAsset(&shader);

        shaderName = "Shader/StandardPBR_VertexPulling.hlsl";
        shader.SetPath(shaderName);
        shader.SetAssetID(AssetIDGenerator::NewFromFile(shader.GetPath()));
        AssetRegistry::GetInstance()->RegisterAsset(&shader);

        shaderName = "Shader/SimpleTestShader.hlsl";
        shader.SetPath(shaderName);
        shader.SetAssetID(AssetIDGenerator::NewFromFile(shader.GetPath()));
        AssetRegistry::GetInstance()->RegisterAsset(&shader);

        shaderName = "Shader/StandardPBR.hlsl";
        shader.SetPath(shaderName);
        shader.SetAssetID(AssetIDGenerator::NewFromFile(shader.GetPath()));
        AssetRegistry::GetInstance()->RegisterAsset(&shader);
        
        shaderName = "Shader/StandardPBR_VertexPulling.hlsl";
        shader.SetPath(shaderName);
        shader.SetAssetID(AssetIDGenerator::NewFromFile(shader.GetPath()));
        AssetRegistry::GetInstance()->RegisterAsset(&shader);

    }

    void BistroSceneLoader::CreateDefaultResources()
    {
        const std::string defaultTexturePath = "Textures/DefaultWhite.dds";
        const std::string defaultMaterialPath = "Material/Default.mat";

        std::string textureFullPath = PathSettings::ResolveAssetPath(defaultTexturePath);
        std::ifstream textureIn(textureFullPath, std::ios::binary);
        bool textureExists = textureIn.good();
        textureIn.close();

        if (!textureExists)
        {
            DDSHeader header = {};
            header.magic = 0x20534444; // "DDS "
            header.fileSize = 124;
            header.flags = 0x00081007;
            header.height = 2;
            header.width = 2;
            header.pitchOrLinearSize = 8;
            header.depth = 0;
            header.mipMapCount = 1;
            header.size = 32;
            header.flagsData = 0x4;
            header.fourCC = 0x31545844; // "DXT1"
            header.rgbBitCount = 0;
            header.rBitMask = 0;
            header.gBitMask = 0;
            header.bBitMask = 0;
            header.aBitMask = 0;
            header.caps = 0x1000;
            header.caps2 = 0;
            header.caps3 = 0;
            header.caps4 = 0;
            header.reserved2 = 0;

            std::ofstream textureOut(textureFullPath, std::ios::binary);
            if (textureOut.is_open())
            {
                textureOut.write(reinterpret_cast<const char*>(&header), sizeof(header));

                uint16_t color = 0xFFFF;
                uint32_t indices = 0;
                textureOut.write(reinterpret_cast<const char*>(&color), sizeof(color));
                textureOut.write(reinterpret_cast<const char*>(&color), sizeof(color));
                textureOut.write(reinterpret_cast<const char*>(&indices), sizeof(indices));
            }
        }

        {
            Texture tex;
            tex.SetAssetCreateMethod(AssetCreateMethod::Serialization);
            tex.SetPath(defaultTexturePath);
            tex.SetAssetID(AssetIDGenerator::NewFromFile(defaultTexturePath));
            AssetRegistry::GetInstance()->RegisterAsset(&tex);
        }

        std::string materialFullPath = PathSettings::ResolveAssetPath(defaultMaterialPath);
        std::ifstream materialIn(materialFullPath, std::ios::binary);
        bool materialExists = materialIn.good();
        materialIn.close();

        if (!materialExists)
        {
            std::string shaderName = "Shader/StandardPBR.hlsl";
            if (RenderSettings::s_EnableVertexPulling)
            {
                shaderName = "Shader/StandardPBR_VertexPulling.hlsl";
            }

            ResourceHandle<Shader> pbrShader = ResourceManager::GetInstance()->LoadAsset<Shader>(shaderName);
            if (pbrShader.IsValid())
            {
                pbrShader->name = "StandardPBR";
                AssetID newID = AssetIDGenerator::NewFromFile(defaultMaterialPath);

                Material* material = new Material(pbrShader);
                material->SetPath(defaultMaterialPath);
                material->SetAssetCreateMethod(AssetCreateMethod::Serialization);
                material->SetAssetID(newID);
                material->archyTypeName = "StandardPBR";

                Vector4 baseColor(1.0f, 1.0f, 1.0f, 1.0f);
                material->SetValue("DiffuseColor", &baseColor, sizeof(Vector4));

                float metallic = 0.0f;
                material->SetValue("Metallic", &metallic, sizeof(float));

                float roughness = 1.0f;
                material->SetValue("Roughness", &roughness, sizeof(float));

                AssetID defaultTexID = AssetIDGenerator::NewFromFile(defaultTexturePath);
                material->SetTexture("DiffuseTexture", defaultTexID.value);

                MaterialLoader loader;
                loader.SaveMaterialToBin(material, defaultMaterialPath, (uint64_t)newID);
                delete material;
            }
        }

        {
            Material mat;
            mat.SetAssetCreateMethod(AssetCreateMethod::Serialization);
            mat.SetPath(defaultMaterialPath);
            mat.SetAssetID(AssetIDGenerator::NewFromFile(defaultMaterialPath));
            AssetRegistry::GetInstance()->RegisterAsset(&mat);
        }
    }

    AssetID BistroSceneLoader::GetTextureAssetID(const tinygltf::Model& model, int textureIndex)
    {
        // 从 GLTF texture index 转换到 AssetID
        // 1. texture index -> image index
        // 2. image index -> AssetID (通过 m_ImageIndexToID)
        
        if (textureIndex < 0 || textureIndex >= model.textures.size()) {
            ASSERT(false);
            return AssetID(); // 返回无效ID
        }
        
        int imageIndex = model.textures[textureIndex].source;
        
        if (imageIndex < 0 || imageIndex >= m_ImageIndexToID.size()) {
            ASSERT(false);
            return AssetID(); // 返回无效ID
        }
        
        return m_ImageIndexToID[imageIndex];
    }
}

