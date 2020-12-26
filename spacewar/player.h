﻿#pragma once

#include "game_logic.h"

#include <SFML/Window/Keyboard.hpp>
#include <functional>

struct PlayerKeymap
{
    sf::Keyboard::Key left = sf::Keyboard::Unknown;
    sf::Keyboard::Key right = sf::Keyboard::Unknown;
    sf::Keyboard::Key shoot = sf::Keyboard::Unknown;
    sf::Keyboard::Key accelerate = sf::Keyboard::Unknown;
    sf::Keyboard::Key accelerateImpulse = sf::Keyboard::Unknown;
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
    callback(keymap.accelerate);
    callback(keymap.accelerateImpulse);
}

inline ShipInput readPlayerInput(const PlayerKeymap& keymap)
{
    ShipInput result;

    if (sf::Keyboard::isKeyPressed(keymap.left))
    {
        result.steer -= 1.0f;
    }

    if (sf::Keyboard::isKeyPressed(keymap.right))
    {
        result.steer += 1.0f;
    }

    result.shoot = sf::Keyboard::isKeyPressed(keymap.shoot);
    result.accelerate = sf::Keyboard::isKeyPressed(keymap.accelerate);
    result.accelerateImpulse = sf::Keyboard::isKeyPressed(keymap.accelerateImpulse);

    return result;
}
