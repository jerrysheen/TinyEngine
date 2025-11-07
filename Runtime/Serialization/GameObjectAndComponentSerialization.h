#pragma once
#include "json.hpp"
#include "GameObject/GameObject.h"
#include "GameObject/Component.h"
#include "GameObject/Camera.h"
#include "GameObject/ComponentType.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "GameObject/MonoBehaviour.h"
#include "GameObject/Transform.h"


namespace EngineCore
{
    using json = nlohmann::json;
    
    // ComponnetType 
    NLOHMANN_JSON_SERIALIZE_ENUM(EngineCore::ComponentType, {
        {EngineCore::ComponentType::Camera, "Camera"},
        {EngineCore::ComponentType::MeshFilter, "MeshFilter"},
        {EngineCore::ComponentType::MeshRenderer, "MeshRenderer"},
        {EngineCore::ComponentType::Transform, "Transform"},
        {EngineCore::ComponentType::Script, "Script"},
    })

    // Camera Component:
    // Camera Component只需要记录相机本身的参数， ViewMatrix是其Transform记录的
    inline void to_json(json& j, const Camera& v)
    {
        j = json{ 
            {"Fov", v.mFov}, 
            {"Aspect", v.mAspect}, 
            {"Near", v.mNear}, 
            {"Far", v.mFar}, 
            {"Width", v.mWidth}, 
            {"Height", v.mHeight} 
        };
    }

    inline void from_json(const json& j, EngineCore::Camera& v)
    {
        j.at("Fov").get_to(v.mFov);
        j.at("Aspect").get_to(v.mAspect);
        j.at("Near").get_to(v.mNear);
        j.at("Far").get_to(v.mFar);
        j.at("Width").get_to(v.mWidth);
        j.at("Height").get_to(v.mHeight);
    }

    // MeshFilter Component:
    inline void to_json(json& j, const MeshFilter& v)
    {
        j = json{ 
            {"AssetHandle", v.mMeshHandle},
        };
    }

    inline void from_json(const json& j, EngineCore::MeshFilter& v)
    {
        j.at("AssetHandle").get_to(v.mMeshHandle);
    }

    // MeshRenderer Component:
    inline void to_json(json& j, const MeshRenderer& v)
    {
        j = json{ 
            {"AssetHandle", v.mMatHandle},
        };
    }
    
    inline void from_json(const json& j, EngineCore::MeshRenderer& v)
    {
        j.at("AssetHandle").get_to(v.mMatHandle);
    }

    // Transform component：
    // child parent结构在GameObject中维护， 这里只记录单纯transform数据
    inline void to_json(json& j, const EngineCore::Transform& v)
    {
        j = json{ 
            {"LocalPosition", v.GetLocalPosition()},
            {"LocalQuaternion", v.GetLocalQuaternion()},
            {"LocalScale", v.GetLocalScale()},
        };
    }
    
    inline void from_json(const json& j, EngineCore::Transform& v)
    {
        // 先从 json 获取值，再调用 setter
        v.SetLocalPosition(j.at("LocalPosition").get<Vector3>());
        v.SetLocalQuaternion(j.at("LocalQuaternion").get<Quaternion>());
        v.SetLocalScale(j.at("LocalScale").get<Vector3>());
    }

    // MonoBehaviour component:
    // 虚函数实现，每个函数实现自己的SerializeField
    inline void to_json(json& j, const EngineCore::MonoBehaviour& v)
    {
        j = json{ 
            {"ScriptName", v.GetScriptName()},
            {"Fields", v.SerializeFields()},
        };
    }
    
    // GameObject:
    // 一种不同的序列化方式
    inline void to_json(json& j, const EngineCore::GameObject& obj)
    {
        json componentsArray = json::array();

        // 遍历所有组件，手动根据类型调用对应的序列化
        for (const auto& [type, comp] : obj.components)
        {
            json compJson;
            compJson["Type"] = type;  // 先保存类型标识

            // 根据类型强转并序列化
            switch (type) {
            case ComponentType::Camera:
                compJson["Data"] = *static_cast<Camera*>(comp);  // ← 触发 Camera 的 to_json
                break;
            case ComponentType::MeshFilter:
                compJson["Data"] = *static_cast<MeshFilter*>(comp);
                break;
            case ComponentType::MeshRenderer:
                compJson["Data"] = *static_cast<MeshRenderer*>(comp);
                break;
            case ComponentType::Transform:
                compJson["Data"] = *static_cast<Transform*>(comp);
                break;
            case ComponentType::Script:
                compJson["Data"] = *static_cast<MonoBehaviour*>(comp);
                break;
            }

            componentsArray.push_back(compJson);
        }

        // 递归序列化子对象
        json childrenArray = json::array();
        for (GameObject* child : obj.GetChildren()) {  // 或 obj.GetChildren()
            childrenArray.push_back(*child);  // ← 递归调用 GameObject 的 to_json
        }

        j = json{ 
            {"Enabled", obj.enabled},
            {"Name", obj.name},
            {"Component", componentsArray},
            {"ChildGameObject", childrenArray}
        };
    }
    
    inline void from_json(const json& j, EngineCore::GameObject& v)
    {
        //// 先从 json 获取值，再调用 setter
        //v.SetLocalPosition(j.at("LocalPosition").get<Vector3>());
        //v.SetLocalRotation(j.at("LocalQuaternion").get<Quaternion>());
        //v.SetLocalScale(j.at("LocalScale").get<Vector3>());
    }
}