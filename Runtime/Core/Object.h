#pragma once
#include "Core/InstanceID.h"

namespace EngineCore
{
    class Object
    {
    public:
        InstanceID id;
        inline const uint64_t GetInstanceID() const{ return id.v;};

        Object(){ id = InstanceIDGenerator::New();};
    };

}