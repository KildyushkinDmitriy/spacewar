#pragma once

#include "app_state.h"
#include "player.h"

#include <SFML/Graphics.hpp>

void renderGameOverUi(const AppStateGameOver& gameOverState, const std::vector<Player>& players, sf::RenderWindow& window, const sf::Font& font);
void renderStartingUi(const AppStateStarting& startingState, const std::vector<Player>& players, sf::RenderWindow& window, const sf::Font& font);
