#pragma once

#include "game_math.h"

struct PlayerInput
{
    float steer = 0.f;
    bool accelerate = false;
    bool fire = false;
};

struct Ship
{
    Vec2 pos{};
    Vec2 velocity{};
    float acceleration = 50.f;

    float rotation = 0;
    float steeringSpeed = TAU / 4.f;

    PlayerInput input{};
};

struct GameWorld
{
    Vec2 size{};
    Ship ship{};
};
