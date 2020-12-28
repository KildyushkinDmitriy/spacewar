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
    int score = 0;
};

inline void forEachKeyInKeymap(const PlayerKeymap& keymap, const std::function<void(sf::Keyboard::Key)>& callback)
{
    callback(keymap.left);
    callback(keymap.right);
    callback(keymap.shoot);
    callback(keymap.thrust);
    callback(keymap.thrustBurst);
}

inline ShipInput readPlayerInput(const PlayerKeymap& keymap)
{
    ShipInput result;

    if (sf::Keyboard::isKeyPressed(keymap.left))
    {
        result.rotate -= 1.0f;
    }

    if (sf::Keyboard::isKeyPressed(keymap.right))
    {
        result.rotate += 1.0f;
    }

    result.shoot = sf::Keyboard::isKeyPressed(keymap.shoot);
    result.thrust = sf::Keyboard::isKeyPressed(keymap.thrust);
    result.thrustBurst = sf::Keyboard::isKeyPressed(keymap.thrustBurst);

    return result;
}
