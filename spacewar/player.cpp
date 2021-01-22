#include "player.h"

#include "entt.hpp"

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

ShipInput aiGenerateInput(const entt::registry& registry, const entt::registry::entity_type selfShip)
{
    entt::registry::entity_type enemyShip = entt::null;
    {
        const auto shipsView = registry.view<const ShipComponent>();

        for (auto [entity, ship] : shipsView.each())
        {
            if (entity != selfShip)
            {
                enemyShip = entity;
                break;
            }
        }
    }
    if (!registry.valid(enemyShip))
    {
        return {};
    }

    const Vec2 selfPos = registry.get<PositionComponent>(selfShip).vec;
    const Vec2 enemyPos = registry.get<PositionComponent>(enemyShip).vec;
    const float selfRotation = registry.get<RotationComponent>(selfShip).angle;

    const Vec2 vecToEnemy = enemyPos - selfPos;
    const float distToEnemy = vec2Length(vecToEnemy);
    const float angleToEnemy = vec2DirToAngle(vecToEnemy);
    
    ShipInput result;
    
    const float angleDiff = angleToEnemy - selfRotation;
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
