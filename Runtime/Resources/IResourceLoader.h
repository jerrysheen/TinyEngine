#pragma once

#include "Resource.h"
#include <functional>
namespace EngineCore
{
    struct LoadTask;

    struct LoadDependency
    {
        AssetID id;
        AssetType type;
        std::function<void()> onLoaded;
        LoadDependency(AssetID id, AssetType type, std::function<void()> onLoaded)
            : id(id), type(type), onLoaded(onLoaded)
        {
        }
    };

    struct LoadResult
    {
        LoadTask* task;
        Resource* resource;
        std::vector<LoadDependency> dependencyList;
    };

    class IResourceLoader
    {
    public:
        virtual ~IResourceLoader() = default;
        virtual LoadResult Load(const std::string& path) = 0;
    };
}