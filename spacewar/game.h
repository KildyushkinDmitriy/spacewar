#pragma once

#include "game_math.h"
#include <vector>

struct GameplaySettings
{
    float shipAcceleration = 50.f;
    float shipSteeringSpeed = TAU / 4.f;

    float shootCooldown = 1.f;
    float projectileSpeed = 300.f;
    float projectileLifetime = 5.f;
};

struct PlayerInput
{
    float steer = 0.f;
    bool accelerate = false;
    bool fire = false;
};

struct Ship
{
    Vec2 pos{};
    float rotation = 0;

    Vec2 velocity{};

    PlayerInput input{};

    float shootCooldownLeft = 0.f;
};

struct Projectile
{
    Vec2 pos{};
    float rotation = 0;

    Vec2 velocity{};
    
    float lifetimeLeft = 0.f;
};

struct GameWorld
{
    GameplaySettings settings;
    Vec2 size{};
    std::vector<Ship> ships{};
    std::vector<Projectile> projectiles{};
};

void gameUpdate(GameWorld& world, float dt);
