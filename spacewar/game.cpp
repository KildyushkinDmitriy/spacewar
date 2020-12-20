#include "game.h"

#include <algorithm>

static Vec2 calcNewPosition(const Vec2 pos, const Vec2 velocity, float dt, const Vec2 worldSize)
{
    return vec2Wrap(pos + velocity * dt, worldSize);
}

void gameUpdate(GameWorld& world, const float dt)
{
    // Clear dead ships
    // const auto res = std::remove_if(world.ships.begin(), world.ships.end(), [](const Ship& ship)
    // {
    // return ship.isDead;
    // });
    // world.ships.erase(res, world.ships.end());

    const GameplaySettings& settings = world.settings;

    // Ships
    for (Ship& ship : world.ships)
    {
        // Move
        ship.rotation += ship.input.steer * settings.shipSteeringSpeed * dt;
        ship.rotation = floatWrap(ship.rotation, TAU);

        const Vec2 forwardDir{cos(ship.rotation), sin(ship.rotation)};

        if (ship.input.accelerate)
        {
            ship.velocity += forwardDir * settings.shipAcceleration * dt;
        }

        ship.pos = calcNewPosition(ship.pos, ship.velocity, dt, world.size);

        // Shoot
        ship.shootCooldownLeft -= dt;
        if (ship.shootCooldownLeft < 0)
        {
            ship.shootCooldownLeft = 0;

            if (ship.input.fire)
            {
                Projectile projectile;
                projectile.pos = ship.pos + forwardDir * 30.f;
                projectile.rotation = ship.rotation;
                projectile.velocity = forwardDir * settings.projectileSpeed;
                projectile.lifetimeLeft = settings.projectileLifetime;

                world.projectiles.push_back(projectile);

                ship.shootCooldownLeft = settings.shootCooldown;
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

        const Vec2 newPos = calcNewPosition(projectile.pos, projectile.velocity, dt, world.size);

        for (Ship& ship : world.ships)
        {
            if (isSegmentIntersectCircle(projectile.pos, newPos, ship.pos, settings.shipCollisionRadius))
            {
                ship.isDead = true;
                projectilesToDeleteIndices.push_back(projectileIndex);
            }
        }

        projectile.pos = newPos;
    }

    for (size_t index : projectilesToDeleteIndices)
    {
        world.projectiles.erase(world.projectiles.begin() + index);
    }
}
