#pragma once

#include "game_math.h"
#include "entt.hpp"

#include <optional>

struct PositionComponent
{
    Vec2 vec{};
};

struct WrapPositionAroundWorldComponent
{
};

struct VelocityComponent
{
    Vec2 vec{};
};

struct RotationComponent
{
    float angle = 0.f;
};

struct RotationSpeedComponent
{
    float speed = 0.f;
};

struct AccelerateByInputComponent
{
    bool input = false;
    float acceleration = 0.f;
};

struct AccelerateImpulseByInputComponent
{
    bool input = false;
    CooldownTimer cooldownTimer;
    float power = 0.f;
};

struct AccelerateImpulseAppliedOneshotComponent
{
};

struct RotateByInputComponent
{
    float input = 0.f; // [-1, 1] 
    float rotationSpeed = 0.f;
};

struct ShootingComponent
{
    bool input = false;
    CooldownTimer cooldownTimer;

    float projectileBirthOffset = 0.f;
    float projectileSpeed = 0.f;

    // Not really a good solution. If entt supported entity cloning, it could be just a prototype entity
    std::function<entt::registry::entity_type(entt::registry&)> createProjectileFunc;
};

struct CircleColliderComponent
{
    float radius = 0.f;
};

// projectile moves with collision check against colliders
struct ProjectileComponent
{
};

struct CollisionHappenedOneshotComponent
{
};

struct DestroyByCollisionComponent
{
};

struct DestroyTimerComponent
{
    float timeLeft = 0.f;
};

struct GravityWellComponent
{
    float maxRadius = 0.f;
    float maxPower = 0.f;

    float dragRadius = 0.f;
    float dragCoefficient = 0.f;
};

struct SusceptibleToGravityWellComponent
{
};

struct TeleportComponent
{
    float radius = 0.f;
    Vec2 destination{};
    float speedAfterTeleport = 0.f;
};

struct TeleportableComponent
{
};

struct ShipComponent
{
    int playerIndex = -1;
};

struct GameResult
{
    int victoriousPlayerIndex = -1;

    bool isTie() const
    {
        return victoriousPlayerIndex == -1;
    }
};

void applyVelocitySystem(entt::registry& registry, float dt);
void applyRotationSpeedSystem(entt::registry& registry, float dt);
void rotateByInputSystem(entt::registry& registry);
void accelerateByInputSystem(entt::registry& registry, float dt);
void accelerateImpulseSystem(entt::registry& registry, float dt);
void accelerateImpulseAppliedOneshotComponentClearSystem(entt::registry& registry);
void shootingSystem(entt::registry& registry, float dt);
void wrapPositionAroundWorldSystem(entt::registry& registry, Vec2 worldSize);
void projectileMoveSystem(entt::registry& registry, float dt);
void circleVsCircleCollisionSystem(entt::registry& registry);
void destroyByCollisionSystem(entt::registry& registry);
void destroyTimerSystem(entt::registry& registry, float dt);
void gravityWellSystem(entt::registry& registry, float dt);
void teleportSystem(entt::registry& registry);

std::optional<GameResult> tryGetGameResult(const entt::registry& registry, int playersCount);
