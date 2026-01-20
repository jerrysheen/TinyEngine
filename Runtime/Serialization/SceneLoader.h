#pragma once
#include "AssetHeader.h"
#include "StreamHelper.h"
#include "Resources/IResourceLoader.h"
#include "Scene/Scene.h"
#include "Graphics/Mesh.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "Math/Math.h"
#include "Resources/AssetRegistry.h"
#include "Settings/ProjectSettings.h"
namespace EngineCore
{
    struct SceneSerializedNode
    {
        char name[64];         // 固定长度名字
        int32_t parentIndex = -1;
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;

        uint64_t meshID = 0;
        uint32_t materialID = 0;

    };

    class SceneLoader: public IResourceLoader                       
    {
    public:
        virtual ~SceneLoader() = default;
        virtual Resource* Load(const std::string& relativePath) override
        {
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream in(path, std::ios::binary);

            // todo: 确保进来的文件是Scene类型
            in.seekg(sizeof(AssetHeader));

            Scene* scene = new Scene();
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
                        });
                }

            }
            return scene;
        }

        void SaveSceneToBin(const Scene* scene, const std::string& relativePath, uint32_t id)
        {
            ASSERT(scene && scene->allObjList.size() > 0);
            std::string binPath = PathSettings::ResolveAssetPath(relativePath);
            std::ofstream out(binPath, std::ios::binary);

            AssetHeader header;
            header.assetID =id;
            header.type = 0;
            StreamHelper::Write(out, header);

            std::vector<SceneSerializedNode> linearNode;
            std::unordered_map<GameObject*, uint32_t> gameObjectMap;
            for(int i = 0; i < scene->rootObjList.size(); i++)
            {
                GameObject* gameObject = scene->rootObjList[i];
                
                SerilizedNode(gameObject, gameObjectMap, linearNode);
            }
            
            StreamHelper::WriteVector(out, linearNode);
        }
    private:
        void SerilizedNode(GameObject* gameObject, std::unordered_map<GameObject*, uint32_t>& gameObjectMap, std::vector<SceneSerializedNode>& linearNode)
        {
            SceneSerializedNode node;

            // Name Serialization
            memset(node.name, 0, sizeof(node.name));
            const std::string& goName = gameObject->name;
            size_t copyLen = std::min(goName.size(), sizeof(node.name) - 1);
            memcpy(node.name, goName.c_str(), copyLen);

            Transform* parent = gameObject->transform->parentTransform;
            uint64_t meshID = 0;
            MeshFilter* meshfilter = gameObject->GetComponent<MeshFilter>();
            if (meshfilter != nullptr)
            {
                meshID = meshfilter->mMeshHandle->GetAssetID();
            }

            if (parent == nullptr)
            {
                node.parentIndex = -1;
                gameObjectMap[gameObject] = linearNode.size();
            }
            else
            {
                ASSERT(gameObjectMap.count(parent->gameObject) > 0);
                node.parentIndex = gameObjectMap[parent->gameObject];
                gameObjectMap[gameObject] = linearNode.size();
            }
            node.materialID = 0;
            node.meshID = meshID;
            node.position = gameObject->transform->GetLocalPosition();
            node.rotation = gameObject->transform->GetLocalQuaternion();
            node.scale = gameObject->transform->GetLocalScale();
            linearNode.push_back(node);

            for (auto* child : gameObject->GetChildren()) 
            {
                SerilizedNode(child, gameObjectMap, linearNode);
            }
        }
    };

}

