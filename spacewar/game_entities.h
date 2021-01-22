#pragma once

#include "game_logic.h"

entt::registry::entity_type createProjectileEntity(entt::registry& registry);
entt::registry::entity_type createShipEntity(entt::registry& registry, Vec2 position, float rotation, sf::Color color, int playerIndex);
entt::registry::entity_type createGravityWellEntity(entt::registry& registry, const Vec2 worldSize);
void createStarEntities(entt::registry& registry, const Vec2 worldSize);
