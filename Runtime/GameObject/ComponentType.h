#pragma once

namespace EngineCore
{
    enum class ComponentType
    {
        MeshRender,
        MeshFilter,
        Camera,
        Transfrom
    };
    
}

//namespace std {
//    template<>
//    struct hash<EngineCore::ComponentType> {
//        size_t operator()(const EngineCore::ComponentType& a) const noexcept {
//            return hash<EngineCore::ComponentType>{}(a);
//        }
//    };
//}