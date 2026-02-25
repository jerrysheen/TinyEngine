#pragma once
#include "PreCompiledHeader.h"
#include "ComponentType.h"
#include "Component.h"
#include "MeshFilter.h"
#include "MeshRenderer.h"
#include "Core/Object.h"
#include "Transform.h"
#include "MonoBehaviour.h"

namespace EngineCore
{
    class Component;
    class Scene;
    class Transform;
    //class Transform;
    // later we will inherete from Object, now just keep it simple.
    class GameObject : Object
    {
    public:
        GameObject();

        GameObject::GameObject(const std::string& name, Scene* scene);

        ~GameObject();
        void SetParent(const GameObject* gameObject);
        template<typename T>
        inline T* GetComponent() const;
        template<typename T>
        inline T* AddComponent();

        std::vector<GameObject*> GetChildren() const;
    public:
        Transform* transform;
        std::unordered_map<ComponentType, Component*> components;
        std::vector<MonoBehaviour*> scripts;
        std::string name;
        bool enabled = true;
            // 非模板方式
        void AddComponent(Component* compont);
        inline Scene* GetOwnerScene() { return ownerScene; }
        inline void SetOwnerScene(Scene* scene) { ownerScene = scene; }
    private:
        Scene* ownerScene = nullptr;
    };

    template<typename T>
    inline T* GameObject::GetComponent() const
    {
        // const 后不能直接用 conponents[type]， 因为可能会产生修改。
        ComponentType type = T::GetStaticType();
        auto it = components.find(type);
        if(it != components.end())
        {
            return static_cast<T*>(it->second);            
        }
        return nullptr;
    }

    template<typename T>
    inline T* GameObject::AddComponent()
    {
        // todo: 这边确实该用multimap的， 因为原则上MonoBehaviour可以挂多个

        ComponentType type = T::GetStaticType();
        if(components.count(type) > 0)
        {
            return nullptr;
        }
        T* component = new T(this);
        components.try_emplace(type, component);
        if(type == ComponentType::Script)
        {
            scripts.push_back(reinterpret_cast<MonoBehaviour*>(component));
        }
        return component;
    }



}