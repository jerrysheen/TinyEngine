#pragma once
#include "Vector3.h"

namespace EngineCore
{

    class Quaternion
    {
    public:
        Quaternion() = default;
        Quaternion(float x, float y, float z, float w);    
        // todo: implement real Forward
        Vector3 GetForward(){return Vector3(0.0f, 0.0f, -1.0f);};
        Vector3 GetUp(){return Vector3(0.0f, 1.0f, 0.0f);};
        Vector3 GetRight(){return Vector3(1.0f, 0.0f, 0.0f);};
    };


}