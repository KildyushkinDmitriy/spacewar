#include <algorithm>
#include <SFML/Graphics.hpp>

#include "game.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "SFML window");

    sf::Texture texture;
    if (!texture.loadFromFile("images/image.png"))
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
    
    GameWorld world{};

    world.boundary.left = 0;
    world.boundary.top = 0;
    world.boundary.width = window.getSize().x;
    world.boundary.height = window.getSize().y;

    world.ship.pos.x = world.boundary.width / 2.f;
    world.ship.pos.y = world.boundary.height / 2.f;

    // window.setFramerateLimit(60);
    
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
            world.ship.movementInput = {};

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                world.ship.movementInput.x += 1.0f;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                world.ship.movementInput.x -= 1.0f;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                world.ship.movementInput.y -= 1.0f;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                world.ship.movementInput.y += 1.0f;
            }
        }

        // update
        {
            world.ship.velocity += world.ship.movementInput * world.ship.acceleration * dt;
            world.ship.pos += world.ship.velocity * dt;
        }

        // render
        { 
            window.clear();

            // window.draw(sprite);
            // window.draw(text);

            shipShape.setPosition(world.ship.pos);
            window.draw(shipShape);

            window.display();
        }
    }
    
    return EXIT_SUCCESS;
}
