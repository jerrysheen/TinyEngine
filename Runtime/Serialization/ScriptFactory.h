#pragma once
#include <string.h>

namespace EngineCore
{
    class ScriptFactory
    {
    public :
        using CreateFunc = std::function<MonoBehaviour*(GameObject*)>;
        inline static MonoBehaviour* Create(const std::string& scriptName, GameObject* go)
        {
            auto& registry = GetRegistry();
            if(registry.count(scriptName) > 0)
            {
                return registry[scriptName](go);
            }

            ASSERT_MSG(false, "Can't find this script");
            return nullptr;
        }

        inline static void Register(const std::string& scriptName, CreateFunc createFunc)
        {
            GetRegistry()[scriptName] = createFunc;
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
#define REGISTER_SCRIPT(ScriptClass)\
    namespace { \
        struct ScriptClass##_Register { \
            ScriptClass##_Register() { \
                EngineCore::ScriptFactory::Register( #ScriptClass, \
                    [](EngineCore::GameObject* go) -> EngineCore::MonoBehaviour* {\
                        return new EngineCore::ScriptClass(go); \
                    }); \
            } \
        } ScriptClass##_instance; \
    }