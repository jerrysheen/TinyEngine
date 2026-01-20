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
#include "Resources/AssetRegistry.h"
#include "Resources/Asset.h"

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
            Scene* res = static_cast<Scene*>(sceneLoader.Load(binPath));
            // todo： 这个地方有加载时序问题。。
            SceneManager::GetInstance()->SetCurrentScene(res);
            for (auto& gameObject : res->allObjList) 
            {
                if (gameObject->GetComponent<MeshFilter>() != nullptr)
                {
                    if (gameObject->GetComponent<MeshRenderer>() != nullptr) continue;

                    MeshRenderer* mr = gameObject->AddComponent<MeshRenderer>();

                    // Re-use common material logic
                    std::string shaderName = "Shader/StandardPBR.hlsl";
                    if (RenderSettings::s_EnableVertexPulling)
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
                        mr->TryAddtoBatchManager();
                    }
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
                mr->TryAddtoBatchManager();
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
        if (version != 1) {
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

        for (size_t i = 0; i < scene.nodes.size(); i++) {
            ProcessNode(model.nodes[scene.nodes[i]], model, rootGo, newScene);
        }

        AssetRegistry::GetInstance()->SaveToDisk("AssetRegistry.bin");

        // Save serialized scene
        EngineCore::SceneLoader sceneLoader;
        // Use a temporary ID for now, or fetch from registry if available
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
        std::vector<ResourceHandle<EngineCore::Mesh>> modelHandles;

        // Check Cache
        auto it = m_MeshCache.find(meshIndex);
        if (it != m_MeshCache.end()) {
            modelHandles = it->second;
        } else {
            const tinygltf::Mesh& mesh = model.meshes[meshIndex];

            // Iterate all Primitives
            for (const auto& primitive : mesh.primitives) {
                ResourceHandle<EngineCore::Mesh> modelHandle = ResourceManager::GetInstance()->CreateResource<EngineCore::Mesh>();
                EngineCore::Mesh* mesh = modelHandle.Get();
                mesh->bounds = AABB();

                // 1. Get Accessors and Buffers
                // Indices
                if (primitive.indices > -1) {
                    const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                    const tinygltf::BufferView& bufferView = model.bufferViews[indexAccessor.bufferView];
                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                    const unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + indexAccessor.byteOffset;
                    size_t count = indexAccessor.count;
                    int componentType = indexAccessor.componentType; 

                    mesh->index.reserve(count);
                    if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        const unsigned short* buf = reinterpret_cast<const unsigned short*>(dataStart);
                        for (size_t i = 0; i < count; i++) mesh->index.push_back(buf[i]);
                    } else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                        const unsigned int* buf = reinterpret_cast<const unsigned int*>(dataStart);
                        for (size_t i = 0; i < count; i++) mesh->index.push_back(buf[i]);
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

                    mesh->vertex.resize(count);
                    for (size_t i = 0; i < count; i++) {
                        const float* buf = reinterpret_cast<const float*>(dataStart + i * stride);
                        mesh->vertex[i].position = Vector3(buf[0], buf[1], buf[2]);
                        mesh->bounds.Encapsulate(mesh->vertex[i].position);
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
                        mesh->vertex[i].normal = Vector3(buf[0], buf[1], buf[2]);
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
                        mesh->vertex[i].uv = Vector2(buf[0], buf[1]);
                    }
                }

                // Set Layout
                mesh->layout.push_back(InputLayout(VertexAttribute::POSITION, 3 * sizeof(float), 3, 8 * sizeof(float), 0));
                mesh->layout.push_back(InputLayout(VertexAttribute::NORMAL, 3 * sizeof(float), 3, 8 * sizeof(float), 3 * sizeof(float)));
                mesh->layout.push_back(InputLayout(VertexAttribute::UV0, 2 * sizeof(float), 2, 8 * sizeof(float), 6 * sizeof(float)));

                // Upload to GPU
                mesh->UploadMeshToGPU();

                 {
                     std::string meshName = "Bistro_Mesh_" + std::to_string(meshIndex) + "_" + std::to_string(modelHandles.size());
                     std::string assetPath = "Mesh/" + meshName + ".bin";

                     AssetID runtimeID = mesh->GetAssetID();
                     AssetID newID = AssetIDGenerator::NewFromFile(assetPath);

                     mesh->SetPath(assetPath);
                     mesh->SetAssetCreateMethod(AssetCreateMethod::Serialization);
                     mesh->SetAssetID(newID);

                     ResourceManager::GetInstance()->mResourceCache.erase(runtimeID);
                     ResourceManager::GetInstance()->mResourceCache[newID] = mesh;
                     ResourceManager::GetInstance()->mPathToID[assetPath] = newID;

                     modelHandle = ResourceHandle<EngineCore::Mesh>(newID);

                     MeshLoader loader;
                     loader.SaveMeshToBin(mesh, assetPath, (uint32_t)newID);

                     AssetRegistry::GetInstance()->RegisterAsset(mesh);
                 }

                // Optional: If we want to save memory and don't need CPU copy anymore, we could clear vectors here
                // However, keep it for now as it might be needed for physics/picking
                // modelData->vertex.clear();
                // modelData->vertex.shrink_to_fit();
                // modelData->index.clear();
                // modelData->index.shrink_to_fit();

                modelHandles.push_back(modelHandle);
            }
            
            // Add to cache
            m_MeshCache[meshIndex] = modelHandles;
        }

        if (modelHandles.empty()) return;

        // Create GameObject components
        if (modelHandles.size() == 1) {
            MeshFilter* mf = go->AddComponent<MeshFilter>();
            mf->mMeshHandle = modelHandles[0];

            MeshRenderer* mr = go->AddComponent<MeshRenderer>();
            
            std::string shaderName = "Shader/StandardPBR.hlsl";
            if(RenderSettings::s_EnableVertexPulling)
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
                mr->TryAddtoBatchManager();
            } 
        } else {
            // Multiple primitives become child objects
            for (size_t i = 0; i < modelHandles.size(); i++) {
                GameObject* subGo = targetScene->CreateGameObject(go->name + "_SubMesh_" + std::to_string(i));
                subGo->SetParent(go);

                MeshFilter* mf = subGo->AddComponent<MeshFilter>();
                mf->mMeshHandle = modelHandles[i];

                MeshRenderer* mr = subGo->AddComponent<MeshRenderer>();
                
                std::string shaderName = "Shader/StandardPBR.hlsl";
            if(RenderSettings::s_EnableVertexPulling)
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
                    mr->TryAddtoBatchManager();
                } 
            }
        }
    }
}