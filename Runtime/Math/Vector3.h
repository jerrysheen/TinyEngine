#pragma once

namespace EngineCore
{
    class Vector3
    {
    public:
    static Vector3 Zero;
    static Vector3 One;
    public:
        float x;
        float y;
        float z;
        Vector3(){x = 0; y = 0; z = 0;};
        Vector3(float x);
        Vector3(float x, float y, float z);
    };
}