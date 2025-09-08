#pragma once
#include "PreCompiledHeader.h"
#include "ComponentType.h"
#include "Component.h"
#include "MeshFilterComponent.h"


namespace EngineCore
{
    class Component;
    //class Transform;
    // later we will inherete from Object, now just keep it simple.
    class GameObject
    {
    public:
        GameObject(){};
        ~GameObject(){};
        int m_Instance;
        template<typename T>
        inline T* GetComponent() const;
        template<typename T>
        inline bool AddComponent();
    private:
        std::multimap<ComponentType, Component*> components;
    };

    template<typename T>
    inline T* GameObject::GetComponent() const
    {
        ComponentType type = T::GetType();
        auto iter = components.find(type);
        if(iter != components.end())
        {
             return static_cast<T*>(iter->second);
        }
        return nullptr;
    }

    template<typename T>
    inline bool GameObject::AddComponent()
    {
        T* component = new T();
        ComponentType type = T::GetType();
        components.insert(std::pair<ComponentType, Component*>(type, component));
        return true;
    }

}