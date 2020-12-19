#pragma once

#include <cmath>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

using Vec2f = sf::Vector2f;
using Rectf = sf::FloatRect;

constexpr float PI = 3.14159265359;
constexpr float TAU = PI * 2; // full turn in radians

inline float radToDeg(float rad)
{
    return rad / PI * 180.f;
}

struct PlayerInput
{
    float steer = 0.f;
    bool accelerate = false;
    bool fire = false;
};

struct Ship
{
    Vec2f pos{};
    float rotation = 0;

    Vec2f velocity{};
    float acceleration = 50.f;
    float steeringSpeed = TAU / 4.f;

    PlayerInput input{};
};

struct GameWorld
{
    Rectf boundary{};
    Ship ship{};
};
