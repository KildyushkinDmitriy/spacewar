#include "game_logic.h"

#include <SFML/Graphics.hpp>

static void renderObj(const Vec2 pos, const float rotation, const Vec2& worldSize, sf::Shape& renderShape,
                      sf::RenderWindow& window)
{
    // sprite is pointing upwards, but with zero rotation it must be pointing right, so offset the rotation
    renderShape.setRotation(radToDeg(rotation + TAU / 4.f));

    // draw obj 9 times for wrap around world case, hoping sfml will do the culling 
    {
        renderShape.setPosition(pos);
        window.draw(renderShape);

        renderShape.setPosition(pos + Vec2{worldSize.x, 0.f});
        window.draw(renderShape);

        renderShape.setPosition(pos + Vec2{-worldSize.x, 0.f});
        window.draw(renderShape);

        renderShape.setPosition(pos + Vec2{0.f, worldSize.y});
        window.draw(renderShape);

        renderShape.setPosition(pos + Vec2{0.f, -worldSize.y});
        window.draw(renderShape);

        renderShape.setPosition(pos + worldSize);
        window.draw(renderShape);

        renderShape.setPosition(pos - worldSize);
        window.draw(renderShape);

        renderShape.setPosition(pos + Vec2{-worldSize.x, worldSize.y});
        window.draw(renderShape);

        renderShape.setPosition(pos + Vec2{worldSize.x, -worldSize.y});
        window.draw(renderShape);
    }
}

void renderGame(const GameWorld& world, sf::RenderWindow& window, sf::Texture& shipTexture)
{
    sf::RectangleShape shipShape;
    {
        const Vec2 size{35, 35};
        shipShape.setSize(size);
        shipShape.setOrigin(size / 2.f);
        shipShape.setTexture(&shipTexture);
    }

    sf::RectangleShape projectileShape;
    {
        const Vec2 size{10, 10};
        projectileShape.setSize(size);
        projectileShape.setOrigin(size / 2.f);
        projectileShape.setTexture(&shipTexture);
    }

    sf::CircleShape gravityWellShape{10.f};
    gravityWellShape.setOrigin(Vec2{gravityWellShape.getRadius(), gravityWellShape.getRadius()});
    gravityWellShape.setFillColor(sf::Color::Cyan);
    gravityWellShape.setPosition(world.size / 2.f);
    window.draw(gravityWellShape);

    bool isSecondShip = false;
    for (const Ship& ship : world.ships)
    {
        if (isSecondShip)
        {
            shipShape.setFillColor(sf::Color::Cyan);
        }
        else
        {
            shipShape.setFillColor(sf::Color::White);
        }

        if (ship.isDead)
        {
            shipShape.setFillColor(sf::Color::Blue);
        }

        renderObj(ship.pos, ship.rotation, world.size, shipShape, window);

        isSecondShip = true;
    }

    for (const Projectile& projectile : world.projectiles)
    {
        renderObj(projectile.pos, projectile.rotation, world.size, projectileShape, window);
    }
}