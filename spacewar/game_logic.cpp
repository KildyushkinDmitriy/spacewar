#include "game_logic.h"

#include <cassert>
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

std::optional<GameResult> gameSimulate(GameWorld& world, const float dt)
{
    const GameplaySettings& settings = world.settings;

    // Ships
    for (Ship& ship : world.ships)
    {
        if (ship.isDead)
        {
            continue;
        }

        // Move
        ship.rotation += ship.input.steer * settings.shipSteeringSpeed * dt;
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

        if (ship.input.accelerate)
        {
            ship.velocity += forwardDir * settings.shipAcceleration * dt;
        }

        ship.accelerateImpulseCooldownLeft -= dt;
        if (ship.accelerateImpulseCooldownLeft <= 0)
        {
            ship.accelerateImpulseCooldownLeft = 0;

            if (ship.input.accelerateImpulse)
            {
                ship.velocity += forwardDir * settings.shipAccelerationImpulse;
                ship.accelerateImpulseCooldownLeft = settings.shipAccelerationImpulseCooldown;
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
                ship1.isDead = true;
                ship2.isDead = true;
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
                    ship.velocity = ship.velocity / length * 10.f;
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
        for (Ship& ship : world.ships)
        {
            if (isSegmentIntersectCircle(projectile.pos, newPos, ship.pos, settings.shipCollisionRadius))
            {
                ship.isDead = true;
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

    if (deadShipsCount == 0)
    {
        return {};
    }

    if (deadShipsCount == world.ships.size())
    {
        return GameResult{-1};
    }

    assert(aliveShipIndex != -1);
    return GameResult{aliveShipIndex};
}
