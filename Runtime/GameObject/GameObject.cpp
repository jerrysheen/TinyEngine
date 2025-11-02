#include "PreCompiledHeader.h"
#include "GameObject.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Transform.h"

namespace EngineCore
{
    GameObject::GameObject()
    {
        transform = this->AddComponent<Transform>();
    }

    GameObject::GameObject(const std::string &name, Scene* scene) : name(name)
    {
        ASSERT(scene != nullptr);
        ownerScene = scene;
        transform = this->AddComponent<Transform>();
        ownerScene->AddGameObjectToSceneList(this);
    }

    void GameObject::SetParent(const GameObject* gameObject)
    {
        if (gameObject == nullptr) 
        {
            // 回归到根节点。
            transform->SetParent(nullptr);
            ownerScene->AddRootGameObject(this);
        }
        else 
        {
            transform->SetParent(gameObject->transform);
            ownerScene->TryRemoveRootGameObject(this);
            // 放入某个节点， 清除scene中的GameObject
        }
    }

    GameObject::~GameObject()
    {
        ownerScene->RemoveGameObjectToSceneList(this);
        // todo : component.OnDestroy;
        //todo: 调用所有Component的OnDestory函数
        for(auto& [key, value] : components)
        {
            if(value != nullptr)
            {
                delete value;
                // 注意，不会自动更新value值
                value = nullptr;
            }
        }
        components.clear();
        scripts.clear();

        // 这个地方设计到，GameObject怎么销毁？ 如果都通过SceneManager来销毁，那这个
        ownerScene = nullptr;
    }

} // namespace EngineCore
