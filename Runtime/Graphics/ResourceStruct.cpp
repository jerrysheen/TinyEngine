#include "PreCompiledHeader.h"
#include "ResourceStruct.h"

namespace EngineCore
{

    MaterialStruct* Resources::LoadMaterial(const string& path)
    {
        // todo: 解析Matea数据；
        MaterialStruct* matStruct = new MaterialStruct();
        matStruct->name = "unlit";
        matStruct->path = "fakepath";
        matStruct->shaderCode = "...";
        matStruct->vec3Data.emplace("Color", Vector3(0.3,0.4,0.5));

        return matStruct;
    }
    
}