#pragma once

#include "Resource.h"

namespace EngineCore
{
    class IResourceLoader
    {
    public:
        virtual ~IResourceLoader() = default;
        virtual Resource* Load(const std::string& path) = 0;
    };
}