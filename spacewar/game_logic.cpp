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

        const Vec2 forwardDir = vec2RotationToDir(ship.rotation);

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
        world.projectiles.erase(world.projectiles.begin() + projectilesToDeleteIndices[i]);
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
