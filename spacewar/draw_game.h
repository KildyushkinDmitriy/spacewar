#pragma once

#include "game_logic.h"

#include <SFML/Graphics.hpp>

void drawGame(sf::RenderWindow& window, const sf::Texture& shipTexture, const entt::registry& registry, Vec2 worldSize, float time);
void drawGameDebug(const entt::registry& registry, sf::RenderWindow& window, const sf::Font& font);
