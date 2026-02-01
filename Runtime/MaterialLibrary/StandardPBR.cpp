#include "PreCompiledHeader.h"
#include "StandardPBR.h"
#include "MaterialArchetypeRegistry.h"

namespace Mat::StandardPBR
{
    static bool s_RegistThisMat = [=]()
        {
            return EngineCore::MaterialArchetypeRegistry::GetInstance().RegisMaterial
            (
                GetArchetypeName(),
                GetMaterialLayout()
            );
        }();
    // 立即调用lambda 返回
}