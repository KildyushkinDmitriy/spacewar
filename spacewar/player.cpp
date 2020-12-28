#include "player.h"

void forEachKeyInKeymap(const PlayerKeymap& keymap, const std::function<void(sf::Keyboard::Key)>& callback)
{
    callback(keymap.left);
    callback(keymap.right);
    callback(keymap.shoot);
    callback(keymap.thrust);
    callback(keymap.thrustBurst);
}

ShipInput readPlayerInput(const PlayerKeymap& keymap)
{
    ShipInput result;

    if (sf::Keyboard::isKeyPressed(keymap.left))
    {
        result.rotate -= 1.0f;
    }

    if (sf::Keyboard::isKeyPressed(keymap.right))
    {
        result.rotate += 1.0f;
    }

    result.shoot = sf::Keyboard::isKeyPressed(keymap.shoot);
    result.thrust = sf::Keyboard::isKeyPressed(keymap.thrust);
    result.thrustBurst = sf::Keyboard::isKeyPressed(keymap.thrustBurst);

    return result;
}

ShipInput aiGenerateInput(const GameWorld& world, const int selfShipIndex, const int enemyShipIndex)
{
    const Ship& selfShip = world.ships[selfShipIndex];
    const Ship& enemyShip = world.ships[enemyShipIndex];

    const Vec2 vecToEnemy = enemyShip.pos - selfShip.pos;
    const float distToEnemy = vec2Length(vecToEnemy);
    const float angleToEnemy = vec2DirToAngle(vecToEnemy);

    ShipInput result;

    const float angleDiff = angleToEnemy - selfShip.rotation;
    const float angleDiffAbs = std::abs(angleDiff);

    result.rotate = angleDiff > 0 ? 1.f : -1.f;

    if (angleDiffAbs < 40.f)
    {
        result.thrust = distToEnemy > 150.f;
        result.thrustBurst = distToEnemy > 600.f;
    }

    result.shoot = angleDiffAbs < 10.f;

    return result;
}
