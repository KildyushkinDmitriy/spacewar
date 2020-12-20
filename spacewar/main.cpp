#include <algorithm>
#include <SFML/Graphics.hpp>

#include "game.h"

void runTests();

struct PlayerKeymap
{
    sf::Keyboard::Key left = sf::Keyboard::Unknown;
    sf::Keyboard::Key right = sf::Keyboard::Unknown;
    sf::Keyboard::Key shoot = sf::Keyboard::Unknown;
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

    result.shoot = sf::Keyboard::isKeyPressed(keymap.shoot);
    result.accelerate = sf::Keyboard::isKeyPressed(keymap.accelerate);

    return result;
}

void renderObj(const Vec2 pos, const float rotation, const Vec2& worldSize, sf::Shape& renderShape,
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

GameWorld createWorld(const Vec2 size)
{
    GameWorld world{};
    world.size = size;
    world.ships.resize(2);
    world.ships[0].pos = world.size / 2.f + size / 4.f;
    world.ships[1].pos = world.size / 2.f - size / 4.f;

    world.settings.shipAcceleration = 50.f;
    world.settings.shipSteeringSpeed = TAU / 4.f;
    world.settings.shootCooldown = 1.f;
    world.settings.projectileSpeed = 300.f;
    world.settings.projectileLifetime = 5.f;
    world.settings.shipCollisionRadius = 20.f;
    world.settings.muzzleExtraOffset = 5.f;

    return world;
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

    GameWorld world = createWorld(Vec2{window.getSize()});

    sf::RectangleShape shipShape;
    {
        const Vec2 size{50, 50};
        shipShape.setSize(size);
        shipShape.setOrigin(size / 2.f);
        shipShape.setTexture(&texture);
    }

    sf::RectangleShape projectileShape;
    {
        const Vec2 size{10, 10};
        projectileShape.setSize(size);
        projectileShape.setOrigin(size / 2.f);
        projectileShape.setTexture(&texture);
    }

    sf::CircleShape shipCollisionShape{world.settings.shipCollisionRadius};
    {
        shipCollisionShape.setOrigin(Vec2{shipCollisionShape.getRadius(), shipCollisionShape.getRadius()});
        shipCollisionShape.setFillColor(sf::Color::Red);
    }

    PlayerKeymap player1Keymap;
    player1Keymap.left = sf::Keyboard::A;
    player1Keymap.right = sf::Keyboard::D;
    player1Keymap.shoot = sf::Keyboard::W;
    player1Keymap.accelerate = sf::Keyboard::S;

    PlayerKeymap player2Keymap;
    player2Keymap.left = sf::Keyboard::J;
    player2Keymap.right = sf::Keyboard::L;
    player2Keymap.shoot = sf::Keyboard::I;
    player2Keymap.accelerate = sf::Keyboard::K;

    sf::Clock timer;
    while (window.isOpen())
    {
        float dt = timer.restart().asSeconds();
        dt = std::clamp(dt, 0.f, 0.1f);

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            {
                world = createWorld(Vec2{window.getSize()});
            }

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
                shipShape.setFillColor(sf::Color::White);
                if (ship.isDead)
                {
                    shipShape.setFillColor(sf::Color::Blue);
                }

                renderObj(ship.pos, ship.rotation, world.size, shipShape, window);

                // shipCollisionShape.setPosition(ship.pos);
                // window.draw(shipCollisionShape);
            }

            for (const Projectile& projectile : world.projectiles)
            {
                renderObj(projectile.pos, projectile.rotation, world.size, projectileShape, window);
            }

            // window.draw(sprite);
            // window.draw(text);

            window.display();
        }
    }

    return EXIT_SUCCESS;
}
