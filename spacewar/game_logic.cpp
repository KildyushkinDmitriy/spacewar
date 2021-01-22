#include "game_logic.h"

#include <algorithm>

float gameGetGravityWellPowerAtRadius(const GravityWell& well, const float radius)
{
    const float normalized = std::clamp(radius / well.maxRadius, 0.f, 1.f);
    const float powerFactor = 0.0025f / std::pow(normalized + 0.045f, 2.f);
    return powerFactor * well.maxPower;
}

Vec2 gameGetGravityWellVectorAtPoint(const GravityWell& well, const Vec2 point)
{
    const Vec2 diffVec = well.pos - point;
    const float diffLength = vec2Length(diffVec);

    if (diffLength <= 0.001f)
    {
        return Vec2{0.f, 0.f};
    }

    const Vec2 powerDir = diffVec / diffLength;
    const float power = gameGetGravityWellPowerAtRadius(well, diffLength);
    return powerDir * power;
}

float gameGetGravityWellPowerAtRadius(const GravityWellComponent& well, const float radius)
{
    const float normalized = std::clamp(radius / well.maxRadius, 0.f, 1.f);
    const float powerFactor = 0.0025f / std::pow(normalized + 0.045f, 2.f);
    return powerFactor * well.maxPower;
}

Vec2 gameGetGravityWellVectorAtPoint(const GravityWellComponent& well, Vec2 wellPos, const Vec2 point)
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

static void killShip(GameWorld& world, const size_t shipIndex, GameEvents& gameEvents)
{
    Ship& ship = world.ships[shipIndex];
    if (!ship.isDead)
    {
        ship.isDead = true;
        gameEvents.shipDeath.push_back(GameEventShipDeath{shipIndex});
    }
}

GameEvents gameSimulate(GameWorld& world, const float dt)
{
    world.time += dt;

    const GameplaySettings& settings = world.settings;
    GameEvents gameEvents;

    // Ships
    for (size_t shipIndex = 0; shipIndex < world.ships.size(); ++shipIndex)
    {
        Ship& ship = world.ships[shipIndex];
        if (ship.isDead)
        {
            continue;
        }

        // Move
        ship.rotation += ship.input.rotate * settings.shipRotationSpeed * dt;
        ship.rotation = floatWrap(ship.rotation, 360.f);

        // drag + gravity near well
        for (const GravityWell& well : world.gravityWells)
        {
            if (vec2Dist(well.pos, ship.pos) < well.dragRadius)
            {
                ship.velocity -= ship.velocity * dt * settings.gravityWellDragCoefficient;
            }
            ship.velocity += gameGetGravityWellVectorAtPoint(well, ship.pos) * dt;
        }

        const Vec2 forwardDir = vec2AngleToDir(ship.rotation);

        if (ship.input.thrust)
        {
            ship.velocity += forwardDir * settings.shipThrustAcceleration * dt;
        }

        ship.thrustBurstCooldownLeft -= dt;
        if (ship.thrustBurstCooldownLeft <= 0)
        {
            ship.thrustBurstCooldownLeft = 0;

            if (ship.input.thrustBurst)
            {
                ship.velocity += forwardDir * settings.shipThrustBurstImpulse;
                ship.thrustBurstCooldownLeft = settings.shipThrustBurstImpulseCooldown;

                gameEvents.shipThrustBurst.push_back(GameEventShipThrustBurst{shipIndex});
            }
        }

        ship.pos = vec2Wrap(ship.pos + ship.velocity * dt, world.size);

        // Shoot
        ship.shootCooldownLeft -= dt;
        if (ship.shootCooldownLeft < 0)
        {
            ship.shootCooldownLeft = 0;

            if (ship.input.shoot)
            {
                Projectile projectile;
                projectile.pos = ship.pos + forwardDir * (settings.shipCollisionRadius + settings.muzzleExtraOffset);
                projectile.rotation = ship.rotation;
                projectile.velocity = forwardDir * settings.projectileSpeed;
                projectile.lifetimeLeft = settings.projectileLifetime;

                world.projectiles.push_back(projectile);
                gameEvents.projectileCreate.push_back({world.projectiles.size() - 1});

                ship.shootCooldownLeft = settings.shootCooldown;
            }
        }
    }

    // Check each ship pair for collision
    for (size_t i = 0; i < world.ships.size(); ++i)
    {
        for (size_t j = i + 1; j < world.ships.size(); ++j)
        {
            Ship& ship1 = world.ships[i];
            Ship& ship2 = world.ships[j];

            if (ship1.isDead || ship2.isDead)
            {
                continue;
            }

            const float radius = settings.shipCollisionRadius;

            // note: no continuous collision here yet
            if (isCircleIntersectCircle(ship1.pos, radius, ship2.pos, radius))
            {
                killShip(world, i, gameEvents);
                killShip(world, j, gameEvents);
            }
        }
    }

    // Teleport ship on collision with well. Do it strictly after ship/ship collision check
    for (Ship& ship : world.ships)
    {
        for (const GravityWell& well : world.gravityWells)
        {
            if (isPointInsideCircle(ship.pos, well.pos, 10.f))
            {
                ship.pos = world.size;

                const float length = vec2Length(ship.velocity);
                if (length > 0.001f)
                {
                    ship.velocity = ship.velocity / length * 20.f;
                }
            }
        }
    }

    // Projectiles
    std::vector<size_t> projectilesToDeleteIndices{};

    for (size_t projectileIndex = 0; projectileIndex < world.projectiles.size(); ++projectileIndex)
    {
        Projectile& projectile = world.projectiles[projectileIndex];
        projectile.lifetimeLeft -= dt;
        if (projectile.lifetimeLeft <= 0)
        {
            projectilesToDeleteIndices.push_back(projectileIndex);
            continue;
        }

        const Vec2 newPos = projectile.pos + projectile.velocity * dt;

        // test projectile collision against all ships
        for (size_t shipIndex = 0; shipIndex < world.ships.size(); ++shipIndex)
        {
            Ship& ship = world.ships[shipIndex];
            if (isSegmentIntersectCircle(projectile.pos, newPos, ship.pos, settings.shipCollisionRadius))
            {
                killShip(world, shipIndex, gameEvents);
                projectilesToDeleteIndices.push_back(projectileIndex);
                break;
            }
        }

        projectile.pos = vec2Wrap(newPos, world.size);
    }

    for (int i = projectilesToDeleteIndices.size() - 1; i >= 0; --i)
    {
        const size_t projectilesToDeleteIndex = projectilesToDeleteIndices[i];
        world.projectiles.erase(world.projectiles.begin() + projectilesToDeleteIndex);
        gameEvents.projectileDestroyed.push_back({projectilesToDeleteIndex});
    }

    // Game result
    int deadShipsCount = 0;
    int aliveShipIndex = -1;
    for (size_t i = 0; i < world.ships.size(); ++i)
    {
        Ship& ship = world.ships[i];
        if (ship.isDead)
        {
            deadShipsCount++;
        }
        else
        {
            aliveShipIndex = static_cast<int>(i);
        }
    }

    if (deadShipsCount != 0)
    {
        gameEvents.result = GameEventGameResult{deadShipsCount == world.ships.size() ? -1 : aliveShipIndex};
    }

    return gameEvents;
}


// void update(entt::registry &registry) {
//     auto view = registry.view<const position, velocity>();
//
//     // use a callback
//     view.each([](const auto &pos, auto &vel) { /* ... */ });
//
//     // use an extended callback
//     view.each([](const auto entity, const auto &pos, auto &vel) { /* ... */ });
//
//     // use a range-for
//     for(auto [entity, pos, vel]: view.each()) {
//         // ...
//     }
//
//     // use forward iterators and get only the components of interest
//     for(auto entity: view) {
//         auto &vel = view.get<velocity>(entity);
//         // ...
//     }
// }


void integrateVelocitySystem(entt::registry& registry, const float dt)
{
    const auto view = registry.view<PositionComponent, const VelocityComponent>(entt::exclude<ProjectileComponent>);

    for (auto [entity, position, velocity] : view.each())
    {
        position.vec += velocity.vec * dt;
    }
}

void angularSpeedIntegrateSystem(entt::registry& registry, const float dt)
{
    const auto view = registry.view<RotationComponent, const AngularSpeedComponent>();

    for (auto [entity, rotation, angular] : view.each())
    {
        rotation.angle += angular.speed * dt;
    }
}

void rotateByInputSystem(entt::registry& registry, const float dt)
{
    const auto view = registry.view<RotationComponent, const RotateByInputComponent>();

    for (auto [entity, rotation, rotateByInput] : view.each())
    {
        rotation.angle += rotateByInput.input * rotateByInput.rotationSpeed * dt;
        rotation.angle = floatWrap(rotation.angle, 360.f);
    }
}

auto accelerateByInputSystem(entt::registry& registry, const float dt) -> void
{
    const auto view = registry.view<VelocityComponent, const AccelerateByInputComponent, const RotationComponent>();

    for (auto [entity, velocity, accelerateByInput, rotation] : view.each())
    {
        if (accelerateByInput.accelerateInput)
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
        if (shooting.cooldownTimer.updateAndGetIsUsed(dt, shooting.input))
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

void wrapPositionAroundWorldSystem(entt::registry& registry, const Vec2 worldSize)
{
    const auto view = registry.view<PositionComponent, WrapPositionAroundWorldComponent>();

    for (auto [entity, position] : view.each())
    {
        position.vec = vec2Wrap(position.vec, worldSize);
    }
}

void accelerateImpulseSystem(entt::registry& registry, const float dt)
{
    const auto view = registry.view<AccelerateImpulseByInputComponent, VelocityComponent, const RotationComponent>();

    for (auto [entity, accelerateImpulse, velocity, rotation] : view.each())
    {
        if (accelerateImpulse.cooldownTimer.updateAndGetIsUsed(dt, accelerateImpulse.input))
        {
            const Vec2 forwardDir = vec2AngleToDir(rotation.angle);
            velocity.vec += forwardDir * accelerateImpulse.shipThrustBurstImpulse;

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
                registry.emplace<CollisionHappenedComponent>(pjlEnt);
                registry.emplace<CollisionHappenedComponent>(colliderEnt);
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
                registry.emplace<CollisionHappenedComponent>(*i);
                registry.emplace<CollisionHappenedComponent>(*j);
            }
        }
    }
}

void destroyByCollisionSystem(entt::registry& registry)
{
    const auto view = registry.view<const CollisionHappenedComponent>();

    for (auto [entity] : view.each())
    {
        registry.destroy(entity);
    }
}

void destroyTimerSystem(entt::registry& registry, float dt)
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

void gravityWellSystem(entt::registry& registry, float dt)
{
    const auto gravityWellsView = registry.view<const GravityWellComponent, const PositionComponent>();
    const auto affectedEntitiesView = registry.view<
        VelocityComponent, const PositionComponent, const AffectedByGravityWellComponent>();

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
