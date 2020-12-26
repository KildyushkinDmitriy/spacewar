#include "game_logic.h"
#include "player_input.h"

#include <SFML/Graphics.hpp>

void runTests();

void renderGame(const GameWorld& world, sf::RenderWindow& window, sf::Texture& shipTexture);
void renderGameDebug(const GameWorld& world, sf::RenderWindow& window, sf::Font& font);

GameWorld createWorld(const Vec2 size)
{
    GameWorld world;
    world.size = size;

    Ship ship1;
    ship1.pos = world.size / 2.f - size / 4.f;
    ship1.rotation = 180.f + 45.f;
    world.ships.push_back(ship1);

    Ship ship2;
    ship2.pos = world.size / 2.f + size / 4.f;
    ship2.rotation = 45.f;
    world.ships.push_back(ship2);

    GravityWell well;
    well.pos = size / 2.f;
    well.maxRadius = vec2Length(well.pos);
    well.maxPower = 1500.f;
    well.dragRadius = 50.f;
    world.gravityWells.push_back(well);

    world.settings.shipAcceleration = 25.f;
    world.settings.shipAccelerationImpulse = 75.f;
    world.settings.shipAccelerationImpulseCooldown = 3.f;
    world.settings.shipSteeringSpeed = 180.f;
    world.settings.shootCooldown = 1.f;
    world.settings.projectileSpeed = 150.f;
    world.settings.projectileLifetime = 5.f;
    world.settings.shipCollisionRadius = 12.f;
    world.settings.muzzleExtraOffset = 25.f;
    world.settings.gravityWellDragCoefficient = 0.3f;

    return world;
}

int main()
{
    runTests();

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window{sf::VideoMode(1000, 1000), "Spacewars!", sf::Style::Default, settings};

    sf::Texture texture;
    if (!texture.loadFromFile("images/ship.png"))
    {
        return EXIT_FAILURE;
    }

    sf::Font font;
    if (!font.loadFromFile("fonts/arial.ttf"))
    {
        return EXIT_FAILURE;
    }

    GameWorld world = createWorld(Vec2{window.getSize()});

    PlayerKeymap player1Keymap{
        sf::Keyboard::A, sf::Keyboard::D, sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::LShift
    };
    PlayerKeymap player2Keymap{
        sf::Keyboard::J, sf::Keyboard::L, sf::Keyboard::I, sf::Keyboard::K, sf::Keyboard::RShift
    };

    bool isDebugRender = false;

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

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            {
                world = createWorld(Vec2{window.getSize()});
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tilde)
            {
                isDebugRender = !isDebugRender;
            }
        }

        // player input
        if (world.ships.size() > 0)
        {
            world.ships[0].input = readPlayerInput(player1Keymap);
        }
        if (world.ships.size() > 1)
        {
            world.ships[1].input = readPlayerInput(player2Keymap);
        }

        // update
        gameUpdate(world, dt);

        // render
        window.clear();

        if (isDebugRender)
        {
            renderGameDebug(world, window, font);
        }
        else
        {
            renderGame(world, window, texture);
        }

        window.display();
    }

    return EXIT_SUCCESS;
}
