#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

using Vec2f = sf::Vector2f;
using Rectf = sf::FloatRect;

struct Ship
{
    Vec2f pos{};
    // float rotation = 0;
    
    Vec2f velocity{};
    Vec2f movementInput{};

    float acceleration = 50.f;
};

struct GameWorld
{
    Rectf boundary{};
    Ship ship{};
};
