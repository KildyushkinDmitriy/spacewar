#include <algorithm>
#include <SFML/Graphics.hpp>

#include "game.h"

void runTests();

struct PlayerKeymap
{
    sf::Keyboard::Key left = sf::Keyboard::Unknown;
    sf::Keyboard::Key right = sf::Keyboard::Unknown;
    sf::Keyboard::Key fire = sf::Keyboard::Unknown;
    sf::Keyboard::Key accelerate = sf::Keyboard::Unknown;
};

PlayerInput readPlayerInput(const PlayerKeymap& keymap)
{
    PlayerInput result;

    if (sf::Keyboard::isKeyPressed(keymap.left))
    {
        result.steer -= 1.0f;
    }

    if (sf::Keyboard::isKeyPressed(keymap.right))
    {
        result.steer += 1.0f;
    }

    result.fire = sf::Keyboard::isKeyPressed(keymap.fire);
    result.accelerate = sf::Keyboard::isKeyPressed(keymap.accelerate);

    return result;
}

void renderShip(const Ship& ship, const Vec2& worldSize, sf::Shape& renderShape, sf::RenderWindow& window)
{
    // ship sprite is pointing upwards, but with zero rotation it must be pointing right, so offset the rotation
    renderShape.setRotation(radToDeg(ship.rotation + TAU / 4.f));

    // draw ship 9 times for wrap around world case, hoping sfml will do the culling 
    {
        renderShape.setPosition(ship.pos);
        window.draw(renderShape);

        renderShape.setPosition(ship.pos + Vec2{worldSize.x, 0.f});
        window.draw(renderShape);

        renderShape.setPosition(ship.pos + Vec2{-worldSize.x, 0.f});
        window.draw(renderShape);

        renderShape.setPosition(ship.pos + Vec2{0.f, worldSize.y});
        window.draw(renderShape);

        renderShape.setPosition(ship.pos + Vec2{0.f, -worldSize.y});
        window.draw(renderShape);

        renderShape.setPosition(ship.pos + worldSize);
        window.draw(renderShape);

        renderShape.setPosition(ship.pos - worldSize);
        window.draw(renderShape);

        renderShape.setPosition(ship.pos + Vec2{-worldSize.x, worldSize.y});
        window.draw(renderShape);

        renderShape.setPosition(ship.pos + Vec2{worldSize.x, -worldSize.y});
        window.draw(renderShape);
    }
}

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
    world.ships.resize(2);
    world.ships[0].pos = world.size / 2.f + Vec2{-200.f, -200.f};
    world.ships[1].pos = world.size / 2.f + Vec2{200.f, 200.f};

    PlayerKeymap player1Keymap;
    player1Keymap.left = sf::Keyboard::A;
    player1Keymap.right = sf::Keyboard::D;
    player1Keymap.fire = sf::Keyboard::W;
    player1Keymap.accelerate = sf::Keyboard::S;

    PlayerKeymap player2Keymap;
    player2Keymap.left = sf::Keyboard::J;
    player2Keymap.right = sf::Keyboard::L;
    player2Keymap.fire = sf::Keyboard::I;
    player2Keymap.accelerate = sf::Keyboard::K;

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
            world.ships[0].input = readPlayerInput(player1Keymap);
            world.ships[1].input = readPlayerInput(player2Keymap);
        }

        // update
        gameUpdate(world, dt);

        // render
        {
            window.clear();

            for (const Ship& ship : world.ships)
            {
                renderShip(ship, world.size, shipShape, window);
            }

            // window.draw(sprite);
            // window.draw(text);

            window.display();
        }
    }

    return EXIT_SUCCESS;
}
