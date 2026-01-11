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
#include "Graphics/ModelData.h"
#include "GameObject/GameObject.h"

#include "Renderer/RenderAPI.h"
#include "Scene.h"


namespace EngineCore {
    ResourceHandle<Material> BistroSceneLoader::commonMatHandle;


    Scene* BistroSceneLoader::Load(const std::string& path) {
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
        // 创建根节点作为容器
        GameObject* rootGo = newScene->CreateGameObject("BistroRoot");

        // GLTF 可以有多个 Scene，默认使用 defaultScene
        const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
        
        for (size_t i = 0; i < scene.nodes.size(); i++) {
            ProcessNode(model.nodes[scene.nodes[i]], model, rootGo, newScene);
        }

        return newScene;
    }

    void BistroSceneLoader::ProcessNode(const tinygltf::Node& node, const tinygltf::Model& model, GameObject* parent, Scene* targetScene) {
        GameObject* go = targetScene->CreateGameObject(node.name.empty() ? "Node" : node.name);
        go->SetParent(parent);

        // 设置 Transform
        if (node.translation.size() == 3) {
            go->transform->SetLocalPosition(Vector3((float)node.translation[0], (float)node.translation[1], (float)node.translation[2]));
        }
        if (node.rotation.size() == 4) {
            go->transform->SetLocalQuaternion(Quaternion((float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3]));
        }
        if (node.scale.size() == 3) {
            go->transform->SetLocalScale(Vector3((float)node.scale[0], (float)node.scale[1], (float)node.scale[2]));
        }
        
        // 如果有 matrix，需要分解 (这里简化，假设优先使用 T/R/S 或者没有 matrix)
        // tinygltf 不会自动分解 matrix，如果 GLTF 只用了 matrix，这里需要额外的数学库分解
        // Bistro 场景通常使用 T/R/S

        // 处理 Mesh
        if (node.mesh > -1) {
            ProcessMesh(model.meshes[node.mesh], model, go);
        }

        // 递归子节点
        for (size_t i = 0; i < node.children.size(); i++) {
            ProcessNode(model.nodes[node.children[i]], model, go, targetScene);
        }
    }

    void BistroSceneLoader::ProcessMesh(const tinygltf::Mesh& mesh, const tinygltf::Model& model, GameObject* go) {
        // 使用 ResourceManager 创建并注册 ModelData
        ResourceHandle<ModelData> modelHandle = ResourceManager::GetInstance()->CreateResource<ModelData>();
        ModelData* modelData = modelHandle.Get();
        modelData->bounds = AABB();

        // 暂时只处理第一个 Primitive，或者合并 Primitives
        // Bistro 的 Mesh 可能有多个 Primitive，这里简单处理：如果是多个，只取第一个，或者需要创建子物体
        // 为了简化，这里只处理 Primitive 0，或者将所有 Primitive 合并到一个 ModelData (需要材质一致)
        // 正确做法：每个 Primitive 一个 draw call / submesh。
        
        if (mesh.primitives.empty()) return;

        const tinygltf::Primitive& primitive = mesh.primitives[0];
        
        // 1. 获取 Accessors 和 Buffers
        // Indices
        if (primitive.indices > -1) {
            const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& bufferView = model.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

            const unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + indexAccessor.byteOffset;
            int count = indexAccessor.count;
            int componentType = indexAccessor.componentType; // 5123 (unsigned short), 5125 (unsigned int)

            modelData->index.reserve(count);
            if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const unsigned short* buf = reinterpret_cast<const unsigned short*>(dataStart);
                for (int i = 0; i < count; i++) modelData->index.push_back(buf[i]);
            } else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                const unsigned int* buf = reinterpret_cast<const unsigned int*>(dataStart);
                for (int i = 0; i < count; i++) modelData->index.push_back(buf[i]);
            }
        }

        // Position
        if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("POSITION")->second];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
            
            const unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
            int count = accessor.count;
            int stride = accessor.ByteStride(bufferView); 

            modelData->vertex.resize(count);
            for (int i = 0; i < count; i++) {
                const float* buf = reinterpret_cast<const float*>(dataStart + i * stride);
                modelData->vertex[i].position = Vector3(buf[0], buf[1], buf[2]);
                modelData->bounds.Encapsulate(modelData->vertex[i].position);
            }
        }

        // Normal
        if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
            
            const unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
            int count = accessor.count;
            int stride = accessor.ByteStride(bufferView);

            for (int i = 0; i < count; i++) {
                const float* buf = reinterpret_cast<const float*>(dataStart + i * stride);
                modelData->vertex[i].normal = Vector3(buf[0], buf[1], buf[2]);
            }
        }

        // UV0
        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
            
            const unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
            int count = accessor.count;
            int stride = accessor.ByteStride(bufferView);

            for (int i = 0; i < count; i++) {
                const float* buf = reinterpret_cast<const float*>(dataStart + i * stride);
                modelData->vertex[i].uv = Vector2(buf[0], buf[1]);
            }
        }

        // 设置 Layout
        modelData->layout.push_back(InputLayout(VertexAttribute::POSITION, 3 * sizeof(float), 3, 8 * sizeof(float), 0));
        modelData->layout.push_back(InputLayout(VertexAttribute::NORMAL, 3 * sizeof(float), 3, 8 * sizeof(float), 3 * sizeof(float)));
        modelData->layout.push_back(InputLayout(VertexAttribute::UV0, 2 * sizeof(float), 2, 8 * sizeof(float), 6 * sizeof(float)));

        // 上传到 GPU
        RenderAPI::GetInstance()->SetUpMesh(modelData, false);

        // 添加组件
        MeshFilter* mf = go->AddComponent<MeshFilter>();
        mf->mMeshHandle = modelHandle;

        MeshRenderer* mr = go->AddComponent<MeshRenderer>();
        
        ResourceHandle<Shader> pbrShader = ResourceManager::GetInstance()->LoadAsset<Shader>("Shader/StandardPBR.hlsl");
        
        
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
