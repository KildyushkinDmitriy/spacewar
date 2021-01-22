#pragma once

#include "game_logic.h"

#include <SFML/Graphics.hpp>

void drawGame(sf::RenderWindow& window, const sf::Texture& shipTexture, const entt::registry& registry, Vec2 worldSize, float time);
// void renderGameDebug(const GameWorld& world, sf::RenderWindow& window, const sf::Font& font);
