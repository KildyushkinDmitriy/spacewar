#pragma once

#include "app_state.h"
#include "player.h"

#include <SFML/Graphics.hpp>

void drawGameOverUi(const AppStateGameOver& gameOverState, const std::vector<Player>& players, sf::RenderWindow& window, const sf::Font& font);
void drawStartingUi(const AppStateStarting& startingState, const std::vector<Player>& players, sf::RenderWindow& window, const sf::Font& font);
