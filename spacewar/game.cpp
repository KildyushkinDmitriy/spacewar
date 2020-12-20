#include "game.h"

void gameUpdate(GameWorld& world, const float dt)
{
    for (Ship& ship : world.ships)
    {
        ship.rotation += ship.input.steer * ship.steeringSpeed * dt;
        ship.rotation = floatWrap(ship.rotation, TAU);

        if (ship.input.accelerate)
        {
            const Vec2 forwardDir{cos(ship.rotation), sin(ship.rotation)};
            ship.velocity += forwardDir * ship.acceleration * dt;
        }

        ship.pos += ship.velocity * dt;
        ship.pos = vec2Wrap(ship.pos, world.size);
    }
}
