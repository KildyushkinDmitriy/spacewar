#pragma once

#include "game_logic.h"
#include "player.h"

entt::registry::entity_type createProjectileEntity(entt::registry& registry);
entt::registry::entity_type createShipEntity(entt::registry& registry, Vec2 position, float rotation, sf::Color color, int playerIndex);
entt::registry::entity_type createGravityWellEntity(entt::registry& registry, Vec2 worldSize);
void createStarEntities(entt::registry& registry, Vec2 worldSize);
void recreateGameWorld(entt::registry& registry, std::vector<Player>& players, Vec2 worldSize);
