#include <algorithm>
#include <SFML/Graphics.hpp>

#include "game.h"

void runTests();

int main()
{
    runTests();

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "SFML window");

    sf::Texture texture;
    if (!texture.loadFromFile("images/ship.png"))
    {
        return EXIT_FAILURE;
    }
    sf::Sprite sprite(texture);

    sf::Font font;
    if (!font.loadFromFile("fonts/arial.ttf"))
    {
        return EXIT_FAILURE;
    }
    sf::Text text("Hello SFML", font, 50);


    sf::RectangleShape shipShape;
    shipShape.setSize({50, 50});
    shipShape.setOrigin({25, 25});
    shipShape.setTexture(&texture);

    GameWorld world{};
    world.size = Vec2{window.getSize()};
    world.ship.pos = world.size / 2.f;

    sf::Clock timer;
    while (window.isOpen())
    {
        float dt = timer.restart().asSeconds();
        dt = std::clamp(dt, 0.f, 0.1f);

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // input
        {
            world.ship.input = {};

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                world.ship.input.steer -= 1.0f;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                world.ship.input.steer += 1.0f;
            }

            world.ship.input.fire = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
            world.ship.input.accelerate = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
        }

        // update
        {
            world.ship.rotation += world.ship.input.steer * world.ship.steeringSpeed * dt;
            world.ship.rotation = floatWrap(world.ship.rotation, TAU);

            if (world.ship.input.accelerate)
            {
                Vec2 forwardDir{cos(world.ship.rotation), sin(world.ship.rotation)};
                world.ship.velocity += forwardDir * world.ship.acceleration * dt;
            }

            world.ship.pos += world.ship.velocity * dt;
            world.ship.pos = vec2Wrap(world.ship.pos, world.size);
        }

        // render
        {
            window.clear();

            // window.draw(sprite);
            // window.draw(text);

            // ship sprite is pointing upwards, but with zero rotation it must be pointing right, so offset the rotation
            shipShape.setRotation(radToDeg(world.ship.rotation + TAU / 4.f));

            // draw ship 9 times for wrap around world case, hoping sfml will do the culling 
            {
                shipShape.setPosition(world.ship.pos);
                window.draw(shipShape);

                shipShape.setPosition(world.ship.pos + Vec2{world.size.x, 0.f});
                window.draw(shipShape);

                shipShape.setPosition(world.ship.pos + Vec2{-world.size.x, 0.f});
                window.draw(shipShape);

                shipShape.setPosition(world.ship.pos + Vec2{0.f, world.size.y});
                window.draw(shipShape);

                shipShape.setPosition(world.ship.pos + Vec2{0.f, -world.size.y});
                window.draw(shipShape);

                shipShape.setPosition(world.ship.pos + world.size);
                window.draw(shipShape);

                shipShape.setPosition(world.ship.pos - world.size);
                window.draw(shipShape);

                shipShape.setPosition(world.ship.pos + Vec2{-world.size.x, world.size.y});
                window.draw(shipShape);

                shipShape.setPosition(world.ship.pos + Vec2{world.size.x, -world.size.y});
                window.draw(shipShape);
            }

            window.display();
        }
    }

    return EXIT_SUCCESS;
}
