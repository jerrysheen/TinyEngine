#pragma once

namespace EngineCore
{
    class Vector2
    {
    public:
    static Vector2 Zero;
    static Vector2 One;
    public:
        float x;
        float y;
        Vector2(){x = 0; y = 0;};
        Vector2(float x);
        Vector2(float x, float y);
    };
}