#pragma once

#include "game_math.h"
#include <vector>

struct GameplaySettings
{
    float shipAcceleration = 0.f;
    float shipSteeringSpeed = 0.f;
    float shootCooldown = 0.f;
    float projectileSpeed = 0.f;
    float projectileLifetime = 0.f;
    float shipCollisionRadius = 0.f;
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
    float rotation = 0.f;

    Vec2 velocity{};

    PlayerInput input{};

    float shootCooldownLeft = 0.f;

    bool isDead = false;
};

struct Projectile
{
    Vec2 pos{};
    float rotation = 0.f;

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
