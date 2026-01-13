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


namespace EngineCore {
    ResourceHandle<Material> BistroSceneLoader::commonMatHandle;

    Scene* BistroSceneLoader::Load(const std::string& path) {
        BistroSceneLoader loader;
        return loader.LoadInternal(path);
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

        // Cache is cleared when loader instance is destroyed (end of Load function)
        return newScene;
    }

    void BistroSceneLoader::ProcessNode(const tinygltf::Node& node, const tinygltf::Model& model, GameObject* parent, Scene* targetScene) {
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
