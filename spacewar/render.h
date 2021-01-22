#pragma once

#include "game_logic.h"
#include "app_state.h"
#include "player.h"
#include "game_visual.h"

#include <SFML/Graphics.hpp>

void renderGame(const GameWorld& world, const GameVisualWorld& visualWorld, sf::RenderWindow& window, const sf::Texture& shipTexture, const entt::registry& registry);
void renderGameDebug(const GameWorld& world, sf::RenderWindow& window, const sf::Font& font);
void renderGameOverUi(const AppStateGameOver& gameOverState, const std::vector<Player>& players, sf::RenderWindow& window, const sf::Font& font);
void renderStartingUi(const AppStateStarting& startingState, const std::vector<Player>& players, sf::RenderWindow& window, const sf::Font& font);

