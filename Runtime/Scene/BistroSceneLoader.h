#pragma once
#include <string>
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Material.h"

#include <map>
#include "Graphics/ModelData.h"

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
        Scene* LoadInternal(const std::string& path);
        void ProcessNode(const tinygltf::Node& node, const tinygltf::Model& model, GameObject* parent, Scene* targetScene);
        void ProcessMesh(int meshIndex, const tinygltf::Model& model, GameObject* go, Scene* targetScene);

        std::map<int, std::vector<ResourceHandle<ModelData>>> m_MeshCache;
    };
}
