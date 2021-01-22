#pragma once

#include "game_math.h"
#include "entt.hpp"

#include <optional>
#include <vector>

struct GameplaySettings
{
    float shipThrustAcceleration = 0.f;
    float shipThrustBurstImpulse = 0.f;
    float shipThrustBurstImpulseCooldown = 0.f;
    float shipRotationSpeed = 0.f;
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
    float rotate = 0.f;
    bool thrust = false;
    bool thrustBurst = false;
    bool shoot = false;
};

struct Ship
{
    Vec2 pos{};
    float rotation = 0.f;

    Vec2 velocity{};

    ShipInput input{};

    float shootCooldownLeft = 0.f;
    float thrustBurstCooldownLeft = 0.f;

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
    float time = 0.f;
};

struct GameEventGameResult
{
    int victoriousPlayerIndex = -1;

    bool isTie() const
    {
        return victoriousPlayerIndex == -1;
    }
};

struct GameEventShipDeath
{
    size_t shipIndex = 0;
};

struct GameEventShipThrustBurst
{
    size_t shipIndex = 0;
};

struct GameEventProjectileCreated
{
    size_t projectileIndex = 0;
};

struct GameEventProjectileDestroyed
{
    size_t projectileIndex = 0;
};

struct GameEvents
{
    std::vector<GameEventShipDeath> shipDeath{};
    std::vector<GameEventShipThrustBurst> shipThrustBurst{};
    std::vector<GameEventProjectileCreated> projectileCreate{};
    std::vector<GameEventProjectileDestroyed> projectileDestroyed{};
    std::optional<GameEventGameResult> result{};
};

GameEvents gameSimulate(GameWorld& world, float dt);

float gameGetGravityWellPowerAtRadius(const GravityWell& well, float radius);
Vec2 gameGetGravityWellVectorAtPoint(const GravityWell& well, Vec2 point);

struct PositionComponent
{
    Vec2 vec{};
};

struct VelocityComponent
{
    Vec2 vec{};
};

struct RotationComponent
{
    float angle = 0.f;
};

struct DrawUsingShipTextureComponent
{
    sf::Color color{};
};

struct AccelerateByInputComponent
{
    bool accelerateInput = false;
    float acceleration = 0.f;
};

struct AccelerateImpulseByInputComponent
{
    bool input = false;

    CooldownTimer cooldownTimer;

    float shipThrustBurstImpulse = 0.f;
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
};

struct WrapPositionAroundWorldComponent
{
};

struct CircleColliderComponent
{
    float radius = 0.f;
};

// projectile moves with collision check against colliders
struct ProjectileComponent
{
};

struct CollisionHappenedComponent
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

struct AffectedByGravityWellComponent
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

void integrateVelocitySystem(entt::registry& registry, float dt);
void rotateByInputSystem(entt::registry& registry, float dt);
void accelerateByInputSystem(entt::registry& registry, float dt);
void accelerateImpulseSystem(entt::registry& registry, float dt);
void shootingSystem(entt::registry& registry, float dt);
void wrapPositionAroundWorldSystem(entt::registry& registry, Vec2 worldSize);
void projectileMoveSystem(entt::registry& registry, float dt);
void circleVsCircleCollisionSystem(entt::registry& registry);
void destroyByCollisionSystem(entt::registry& registry);
void destroyTimerSystem(entt::registry& registry, float dt);
void gravityWellSystem(entt::registry& registry, float dt);
void teleportSystem(entt::registry& registry);
