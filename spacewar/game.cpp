#include "game.h"

static Vec2 calcNewPosition(const Vec2 pos, const Vec2 velocity, float dt, const Vec2 worldSize)
{
    return vec2Wrap(pos + velocity * dt, worldSize);
}

void gameUpdate(GameWorld& world, const float dt)
{
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
                projectile.pos = ship.pos;
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

    for (size_t i = 0; i < world.projectiles.size(); ++i)
    {
        Projectile& projectile = world.projectiles[i];
        projectile.lifetimeLeft -= dt;
        if (projectile.lifetimeLeft <= 0)
        {
            projectilesToDeleteIndices.push_back(i);
        }

        projectile.pos = calcNewPosition(projectile.pos, projectile.velocity, dt, world.size);
    }

    for (size_t index : projectilesToDeleteIndices)
    {
        world.projectiles.erase(world.projectiles.begin() + index);
    }
}

