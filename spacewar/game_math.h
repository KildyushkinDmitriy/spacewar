#pragma once

#include <SFML/System/Vector2.hpp>
#include <cmath>

using Vec2 = sf::Vector2f;

constexpr float PI = 3.14159265359f;
constexpr float TAU = PI * 2.f; // full turn in radians

inline float radToDeg(const float rad)
{
    return rad / PI * 180.f;
}

inline bool floatEq(const float a, const float b, const float tolerance = 0.0001)
{
    return std::fabs(a - b) <= tolerance;
}

inline float floatWrap(const float val, const float max)
{
    const int whole = static_cast<int>(val / max);
    const float remainder = val - whole * max;
    if (val >= 0.f)
    {
        return remainder;
    }
    return max + remainder;
}

inline Vec2 vec2Wrap(const Vec2 val, const Vec2 max)
{
    return Vec2{floatWrap(val.x, max.x), floatWrap(val.y, max.y)};
}
