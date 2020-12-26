#pragma once

#include "game_logic.h"
#include "app_state.h"

#include <SFML/Graphics.hpp>

void renderGame(const GameWorld& world, sf::RenderWindow& window, sf::Texture& shipTexture);
void renderGameDebug(const GameWorld& world, sf::RenderWindow& window, sf::Font& font);
void renderGameOverUi(const AppState::GameOver& gameOverState, sf::RenderWindow& window, sf::Font& font);

