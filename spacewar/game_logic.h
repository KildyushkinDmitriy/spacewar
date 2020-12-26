#pragma once

#include "game_math.h"

#include <optional>
#include <vector>

struct GameplaySettings
{
    float shipAcceleration = 0.f;
    float shipAccelerationImpulse = 0.f;
    float shipAccelerationImpulseCooldown = 0.f;
    float shipSteeringSpeed = 0.f;
    float shootCooldown = 0.f;
    float projectileSpeed = 0.f;
    float projectileLifetime = 0.f;
    float shipCollisionRadius = 0.f;
    // a place where projectile is born is offset from ship's center (shipCollisionRadius + muzzleExtraOffset), so ship will not insta-kill itself on shoot
    float muzzleExtraOffset = 0.f;
    float gravityWellDragCoefficient = 0.0;
};

struct ShipInput
{
    float steer = 0.f;
    bool accelerate = false;
    bool accelerateImpulse = false;
    bool shoot = false;
};

struct Ship
{
    Vec2 pos{};
    float rotation = 0.f;

    Vec2 velocity{};

    ShipInput input{};

    float shootCooldownLeft = 0.f;
    float accelerateImpulseCooldownLeft = 0.f;

    bool isDead = false;
};

struct Projectile
{
    Vec2 pos{};
    float rotation = 0.f;

    Vec2 velocity{};

    float lifetimeLeft = 0.f;
};

struct GravityWell
{
    Vec2 pos{};
    float maxRadius = 0.f;
    float maxPower = 0.f;
    float dragRadius = 0.f;
};

struct GameWorld
{
    GameplaySettings settings{};
    Vec2 size{};
    std::vector<Ship> ships{};
    std::vector<Projectile> projectiles{};
    std::vector<GravityWell> gravityWells{};
};

struct GameResult
{
    int victoriousPlayerIndex = -1;

    bool isTie() const { return victoriousPlayerIndex == -1; }
};

std::optional<GameResult> gameSimulate(GameWorld& world, float dt);

float gameGetGravityWellPowerAtRadius(const GravityWell& well, float radius);
Vec2 gameGetGravityWellVectorAtPoint(const GravityWell& well, Vec2 point);
