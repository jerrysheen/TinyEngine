#include "PreCompiledHeader.h"
#include "Material.h"

namespace EngineCore
{
    Material::Material(const MaterialStruct* matStruct)
    {
        
    }
    
    
    void Material::InitMaterialData(const MaterialStruct* matStruct)
    {
        *data = *matStruct;
    }


}
