#pragma once

#include "app_state.h"
#include "player.h"

#include <SFML/Graphics.hpp>

void drawGameOverUi(GameResult gameResult, float timeWhenRestart, float timeInState, const std::vector<Player>& players, sf::RenderWindow& window, const sf::Font& font);
void drawStartingUi(const std::vector<bool>& playersReady, const std::vector<Player>& players, sf::RenderWindow& window, const sf::Font& font, float timeInState);
