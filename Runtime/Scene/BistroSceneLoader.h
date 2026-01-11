#pragma once
#include <string>
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Material.h"

namespace tinygltf {
    class Node;
    class Model;
    class Mesh;
}

namespace EngineCore {
    class GameObject;
    class Scene;

    class BistroSceneLoader {
    public:
        static Scene* Load(const std::string& path);
        static ResourceHandle<Material> commonMatHandle;
    private:
        static void ProcessNode(const tinygltf::Node& node, const tinygltf::Model& model, GameObject* parent, Scene* targetScene);
        static void ProcessMesh(const tinygltf::Mesh& mesh, const tinygltf::Model& model, GameObject* go);
    };
}
