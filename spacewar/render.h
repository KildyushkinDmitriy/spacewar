#pragma once

#include "game_logic.h"
#include "app_state.h"
#include "player.h"

#include <SFML/Graphics.hpp>

void renderGame(const GameWorld& world, sf::RenderWindow& window, const sf::Texture& shipTexture);
void renderGameDebug(const GameWorld& world, sf::RenderWindow& window, const sf::Font& font);
void renderGameOverUi(const AppState::GameOver& gameOverState, const std::vector<Player>& players, sf::RenderWindow& window, const sf::Font& font);

