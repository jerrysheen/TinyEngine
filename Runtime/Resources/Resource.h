#pragma once 
#include "Core/Object.h"
#include "Asset.h"
#include "Utils/HashCombine.h"
#include "Serialization/MetaData.h"

namespace EngineCore
{

    class Resource : public Object
    {
        // mRefCount 只能够被ResourceManager类或者Handler修改。
    friend class ResourceManager;
    public :
        virtual ~Resource() = default;
        inline AssetID GetAssetID(){ return mAssetID; };
        inline AssetType GetAssetType() const { return mAssetType; }
        inline void SetAssetID(const AssetID& id) { mAssetID.value = id.value; };
        const string& GetPath() const { return mPath; }
    protected:

        Resource()
        {
            SetAssetID(AssetIDGenerator::NewFromInstanceID(GetInstanceID()));
        };
        
        Resource(AssetType type, const string& path)
            :mAssetType(type), mPath(path){}

        Resource(MetaData* metaData) 
            :mPath(metaData->path), mAssetType(metaData->assetType) 
        {
            SetAssetID(AssetIDGenerator::NewFromFile(mPath));
        };
        string mPath ="";
        AssetID mAssetID = {};
        AssetType mAssetType = AssetType::Default;
        int mRefCount = 0;
    };
}