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

struct ShipInput
{
    float rotate = 0.f;
    bool thrust = false;
    bool thrustBurst = false;
    bool shoot = false;
};

struct Player
{
    PlayerKeymap keymap{};
    std::string name{};
    sf::Keyboard::Key makeAiKey = sf::Keyboard::Unknown;
    bool isAi = false;

    int score = 0;
    entt::registry::entity_type shipEntity = entt::null;
};

void forEachKeyInKeymap(const PlayerKeymap& keymap, const std::function<void(sf::Keyboard::Key)>& callback);
ShipInput readPlayerInput(const PlayerKeymap& keymap);
ShipInput aiGenerateInput(const entt::registry& registry, entt::registry::entity_type selfShip);
