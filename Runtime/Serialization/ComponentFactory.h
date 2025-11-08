#pragma once
#include <string.h>
#include "GameObject/Component.h"


// 注册所有Component类， 运行时根据名字生成对应的Component
namespace EngineCore
{
    class ComponentFactory
    {
    public :
        using CreateFunc = std::function<Component*(GameObject*)>;
        inline static Component* Create(const std::string& componentName, GameObject* go)
        {
            auto& registry = GetRegistry();
            if(registry.count(componentName) > 0)
            {
                return registry[componentName](go);
            }

            ASSERT_MSG(false, "Can't find this script");
            return nullptr;
        }

        inline static void Register(const std::string& componentName, CreateFunc createFunc)
        {
            GetRegistry()[componentName] = createFunc;
        }
    private :

        // 这样写的好处， 不用在cpp调用， 初次调用的时候创建，一定保证时序
        static std::unordered_map<std::string, CreateFunc>& GetRegistry()
        {
            static std::unordered_map<std::string, CreateFunc> registry;
            return registry;
        }
    };
}

// namespace 后没有EngineCore， 因为在每个cpp自己私有的空间中
// 避免类型冲突。 ##表示连接符， #表示string化
// 避免类型冲突。 ##表示连接符， #表示string化
#define REGISTER_SCRIPT(ComponentClass)\
    namespace { \
        struct ComponentClass##_Register { \
            ComponentClass##_Register() { \
                EngineCore::ComponentFactory::Register( #ComponentClass, \
                    [](EngineCore::GameObject* go) -> EngineCore::Component* {\
                        return new EngineCore::ComponentClass(go); \
                    }); \
            } \
        } ComponentClass##_instance; \
    }
