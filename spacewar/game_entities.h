#pragma once

#include "game_logic.h"
#include "player.h"

entt::registry::entity_type createProjectileEntity(entt::registry& registry);
entt::registry::entity_type createShipEntity(entt::registry& registry, Vec2 position, float rotation, sf::Color color, int playerIndex);
entt::registry::entity_type createGravityWellEntity(entt::registry& registry, Vec2 worldSize);
void createStarEntities(entt::registry& registry, Vec2 worldSize);

inline void recreateGameWorld(entt::registry& registry, std::vector<Player>& players, const Vec2 worldSize)
{
    registry.clear();

    const auto shipEntity1 = createShipEntity(registry, worldSize / 2.f - worldSize / 4.f, 180.f + 45.f, sf::Color::Cyan, 0);
    const auto shipEntity2 = createShipEntity(registry, worldSize / 2.f + worldSize / 4.f, 45.f, sf::Color::White, 1);

    createGravityWellEntity(registry, worldSize);
    createStarEntities(registry, worldSize);

    players[0].shipEntity = shipEntity1;
    players[1].shipEntity = shipEntity2;
}

