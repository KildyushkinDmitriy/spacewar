#pragma once

#include "game_logic.h"
#include "game_visual.h"

#include <SFML/Graphics.hpp>

void renderGame(const GameWorld& world, const GameVisualWorld& visualWorld, sf::RenderWindow& window, const sf::Texture& shipTexture, const entt::registry& registry, float time);
// void renderGameDebug(const GameWorld& world, sf::RenderWindow& window, const sf::Font& font);

