#pragma once

#include "game_logic.h"

#include <SFML/Window/Keyboard.hpp>
#include <functional>

struct PlayerKeymap
{
    sf::Keyboard::Key left = sf::Keyboard::Unknown;
    sf::Keyboard::Key right = sf::Keyboard::Unknown;
    sf::Keyboard::Key shoot = sf::Keyboard::Unknown;
    sf::Keyboard::Key thrust = sf::Keyboard::Unknown;
    sf::Keyboard::Key thrustBurst = sf::Keyboard::Unknown;
};

struct Player
{
    PlayerKeymap keymap{};
    std::string name{};
    sf::Keyboard::Key makeAiKey = sf::Keyboard::Unknown;
    
    int score = 0;
    bool isAi = false;
    entt::registry::entity_type shipEntity{};
};

void forEachKeyInKeymap(const PlayerKeymap& keymap, const std::function<void(sf::Keyboard::Key)>& callback);
ShipInput readPlayerInput(const PlayerKeymap& keymap);
ShipInput aiGenerateInput(const GameWorld& world, int selfShipIndex, int enemyShipIndex);
