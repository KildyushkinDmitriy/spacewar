#include "game.h"

void gameUpdate(GameWorld& world, const float dt)
{
    world.ship.rotation += world.ship.input.steer * world.ship.steeringSpeed * dt;
    world.ship.rotation = floatWrap(world.ship.rotation, TAU);

    if (world.ship.input.accelerate)
    {
        const Vec2 forwardDir{cos(world.ship.rotation), sin(world.ship.rotation)};
        world.ship.velocity += forwardDir * world.ship.acceleration * dt;
    }

    world.ship.pos += world.ship.velocity * dt;
    world.ship.pos = vec2Wrap(world.ship.pos, world.size);
}
