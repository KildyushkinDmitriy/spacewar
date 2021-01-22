#include "game_logic.h"

float gameGetGravityWellPowerAtRadius(const GravityWellComponent& well, const float radius)
{
    const float normalized = std::clamp(radius / well.maxRadius, 0.f, 1.f);
    const float powerFactor = 0.0025f / std::pow(normalized + 0.045f, 2.f);
    return powerFactor * well.maxPower;
}

Vec2 gameGetGravityWellVectorAtPoint(const GravityWellComponent& well, const Vec2 wellPos, const Vec2 point)
{
    const Vec2 diffVec = wellPos - point;
    const float diffLength = vec2Length(diffVec);

    if (diffLength <= 0.001f)
    {
        return Vec2{0.f, 0.f};
    }

    const Vec2 powerDir = diffVec / diffLength;
    const float power = gameGetGravityWellPowerAtRadius(well, diffLength);
    return powerDir * power;
}

void wrapPositionAroundWorldSystem(entt::registry& registry, const Vec2 worldSize)
{
    const auto view = registry.view<PositionComponent, WrapPositionAroundWorldComponent>();

    for (auto [entity, position] : view.each())
    {
        position.vec = vec2Wrap(position.vec, worldSize);
    }
}

void applyVelocitySystem(entt::registry& registry, const float dt)
{
    // projectile has it's own move system with collision check
    const auto view = registry.view<PositionComponent, const VelocityComponent>(entt::exclude<ProjectileComponent>);

    for (auto [entity, position, velocity] : view.each())
    {
        position.vec += velocity.vec * dt;
    }
}

void applyRotationSpeedSystem(entt::registry& registry, const float dt)
{
    const auto view = registry.view<RotationComponent, const RotationSpeedComponent>();

    for (auto [entity, rotation, angular] : view.each())
    {
        rotation.angle += angular.speed * dt;
        rotation.angle = floatWrap(rotation.angle, 360.f);
    }
}

void rotateByInputSystem(entt::registry& registry)
{
    const auto view = registry.view<RotationComponent, RotationSpeedComponent, const RotateByInputComponent>();

    for (auto [entity, rotation, angularSpeedComponent, rotateByInput] : view.each())
    {
        angularSpeedComponent.speed = rotateByInput.input * rotateByInput.rotationSpeed;
    }
}

void accelerateByInputSystem(entt::registry& registry, const float dt)
{
    const auto view = registry.view<VelocityComponent, const AccelerateByInputComponent, const RotationComponent>();

    for (auto [entity, velocity, accelerateByInput, rotation] : view.each())
    {
        if (accelerateByInput.input)
        {
            const Vec2 forwardDir = vec2AngleToDir(rotation.angle);
            velocity.vec += forwardDir * accelerateByInput.acceleration * dt;
        }
    }
}

void shootingSystem(entt::registry& registry, const float dt)
{
    const auto view = registry.view<ShootingComponent, const PositionComponent, const RotationComponent>();

    for (auto [entity, shooting, position, rotation] : view.each())
    {
        if (shooting.cooldownTimer.updateAndGetWasUsed(dt, shooting.input))
        {
            const Vec2 forwardDir = vec2AngleToDir(rotation.angle);
            const Vec2 projectilePos = position.vec + forwardDir * shooting.projectileBirthOffset;

            const auto projectileEntity = shooting.createProjectileFunc(registry);

            registry.emplace<PositionComponent>(projectileEntity, projectilePos);
            registry.emplace<RotationComponent>(projectileEntity, rotation);
            registry.emplace<VelocityComponent>(projectileEntity, forwardDir * shooting.projectileSpeed);
        }
    }
}

void accelerateImpulseSystem(entt::registry& registry, const float dt)
{
    const auto view = registry.view<AccelerateImpulseByInputComponent, VelocityComponent, const RotationComponent>();

    for (auto [entity, accelerateImpulse, velocity, rotation] : view.each())
    {
        if (accelerateImpulse.cooldownTimer.updateAndGetWasUsed(dt, accelerateImpulse.input))
        {
            const Vec2 forwardDir = vec2AngleToDir(rotation.angle);
            velocity.vec += forwardDir * accelerateImpulse.power;

            registry.emplace<AccelerateImpulseAppliedOneshotComponent>(entity);
        }
    }
}

void accelerateImpulseAppliedOneshotComponentClearSystem(entt::registry& registry)
{
    const auto view = registry.view<const AccelerateImpulseAppliedOneshotComponent>();

    for (auto [entity] : view.each())
    {
        registry.remove<AccelerateImpulseAppliedOneshotComponent>(entity);
    }
}

void projectileMoveSystem(entt::registry& registry, const float dt)
{
    const auto projectilesView = registry.view<PositionComponent, const VelocityComponent, const ProjectileComponent>();
    const auto collidersView = registry.view<const PositionComponent, const CircleColliderComponent>();

    for (auto [pjlEnt, pjlPos, velocity] : projectilesView.each())
    {
        const Vec2 newPos = pjlPos.vec + velocity.vec * dt;

        for (auto [colliderEnt, colliderPos, circleCollider] : collidersView.each())
        {
            if (isSegmentIntersectCircle(pjlPos.vec, newPos, colliderPos.vec, circleCollider.radius))
            {
                registry.emplace_or_replace<CollisionHappenedOneshotComponent>(pjlEnt);
                registry.emplace_or_replace<CollisionHappenedOneshotComponent>(colliderEnt);
                break;
            }
        }

        pjlPos.vec = newPos;
    }
}

void circleVsCircleCollisionSystem(entt::registry& registry)
{
    const auto view = registry.view<const PositionComponent, const CircleColliderComponent>();

    for (auto i = view.begin(); i != view.end(); ++i)
    {
        auto j = i;
        ++j;
        for (; j != view.end(); ++j)
        {
            auto [pos1, coll1] = view.get(*i);
            auto [pos2, coll2] = view.get(*j);

            // note: no continuous collision here yet
            if (isCircleIntersectCircle(pos1.vec, coll1.radius, pos2.vec, coll2.radius))
            {
                registry.emplace<CollisionHappenedOneshotComponent>(*i);
                registry.emplace<CollisionHappenedOneshotComponent>(*j);
            }
        }
    }
}

void destroyByCollisionSystem(entt::registry& registry)
{
    const auto view = registry.view<const CollisionHappenedOneshotComponent, const DestroyByCollisionComponent>();

    for (auto [entity] : view.each())
    {
        registry.destroy(entity);
    }
}

void destroyTimerSystem(entt::registry& registry, const float dt)
{
    const auto view = registry.view<DestroyTimerComponent>();

    for (auto [entity, timer] : view.each())
    {
        timer.timeLeft -= dt;
        if (timer.timeLeft <= 0)
        {
            registry.destroy(entity);
        }
    }
}

void gravityWellSystem(entt::registry& registry, const float dt)
{
    const auto gravityWellsView = registry.view<const GravityWellComponent, const PositionComponent>();
    const auto affectedEntitiesView = registry.view<
        VelocityComponent, const PositionComponent, const SusceptibleToGravityWellComponent>();

    for (auto [wellEnt, well, wellPos] : gravityWellsView.each())
    {
        for (auto [affectedEnt, velocity, affectedPos] : affectedEntitiesView.each())
        {
            if (vec2Dist(wellPos.vec, affectedPos.vec) < well.dragRadius)
            {
                velocity.vec -= velocity.vec * dt * well.dragCoefficient;
            }
            velocity.vec += gameGetGravityWellVectorAtPoint(well, wellPos.vec, affectedPos.vec) * dt;
        }
    }
}

void teleportSystem(entt::registry& registry)
{
    const auto teleportsView = registry.view<const TeleportComponent, const PositionComponent>();
    const auto teleportablesView = registry.view<PositionComponent, VelocityComponent, TeleportableComponent>();

    for (auto [_, teleport, teleportPos] : teleportsView.each())
    {
        for (auto [_, position, velocity] : teleportablesView.each())
        {
            if (isPointInsideCircle(position.vec, teleportPos.vec, teleport.radius))
            {
                position.vec = teleport.destination;

                const float length = vec2Length(velocity.vec);
                if (length > 0.001f)
                {
                    velocity.vec = velocity.vec / length * teleport.speedAfterTeleport;
                }
            }
        }
    }
}

std::optional<GameResult> tryGetGameResult(const entt::registry& registry, const int playersCount)
{
    int anyAliveShipPlayerIndex = -1;
    int aliveShipsCount = 0;

    const auto view = registry.view<const ShipComponent>();
    for (auto [_, ship] : view.each())
    {
        ++aliveShipsCount;
        anyAliveShipPlayerIndex = ship.playerIndex;
    }

    // tie
    if (aliveShipsCount == 0)
    {
        return GameResult{};
    }

    // win
    if (aliveShipsCount < playersCount)
    {
        assert(anyAliveShipPlayerIndex != -1);
        return GameResult{anyAliveShipPlayerIndex};
    }

    return {};
}
