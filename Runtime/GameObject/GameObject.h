#pragma once
#include "PreCompiledHeader.h"
#include "ComponentType.h"
#include "Component.h"
#include "MeshFilter.h"
#include "MeshRenderer.h"
#include "Core/Object.h"
#include "Transform.h"

namespace EngineCore
{
    class Component;
    //class Transform;
    // later we will inherete from Object, now just keep it simple.
    class GameObject
    {
    public:
        GameObject()
        {
            this->AddComponent<Transform>();
        };
        ~GameObject(){};
        template<typename T>
        inline T* GetComponent() const;
        template<typename T>
        inline T* AddComponent();
    private:
        std::unordered_map<ComponentType, Component*> components;
    };

    template<typename T>
    inline T* GameObject::GetComponent() const
    {
        // const 后不能直接用 conponents[type]， 因为可能会产生修改。
        ComponentType type = T::GetType();
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
        ComponentType type = T::GetType();
        if(components.count(type) > 0)
        {
            return nullptr;
        }
        T* component = new T(this);
        components.try_emplace(type, component);
        return component;
    }

}