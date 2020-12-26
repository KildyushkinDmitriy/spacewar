#include "game.h"

#include <algorithm>
#include <SFML/Graphics.hpp>

void runTests();

struct PlayerKeymap
{
    sf::Keyboard::Key left = sf::Keyboard::Unknown;
    sf::Keyboard::Key right = sf::Keyboard::Unknown;
    sf::Keyboard::Key shoot = sf::Keyboard::Unknown;
    sf::Keyboard::Key accelerate = sf::Keyboard::Unknown;
         sf::Keyboard::Key accelerateImpulse = sf::Keyboard::Unknown;
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
    result.accelerateImpulse = sf::Keyboard::isKeyPressed(keymap.accelerateImpulse);

    return result;
}

void drawThickLine(const Vec2 a, const Vec2 b, const float thickness, const sf::Color color, sf::RenderWindow& window)
{
    const Vec2 diff = b - a;
    const float angle = std::atan2f(diff.y, diff.x);

    sf::RectangleShape rectShape{Vec2{vec2Length(diff), thickness}};
    rectShape.setPosition(a);
    rectShape.setOrigin(Vec2{0.f, thickness / 2.f});
    rectShape.rotate(radToDeg(angle));
    rectShape.setFillColor(color);
    window.draw(rectShape);
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

void renderGameDebugView(const GameWorld& world, sf::RenderWindow& window, sf::Font& font)
{
    // gravitation field visualize
    for (const GravityWell& well : world.gravityWells)
    {
        const int gradationsCount = 20;
        const float radiusIncrement = well.maxRadius / (gradationsCount - 1);
        const Vec2 dir = vec2RotationToDir(TAU / 8.f * 5.f);

        struct Measure
        {
            Vec2 point{};
            float radius = 0.f;
            float gravityPower = 0.f;
        };

        std::vector<Measure> measures;

        for (int i = gradationsCount - 1; i >= 0; --i)
        {
            const float radius = i * radiusIncrement;
            const Vec2 point = well.pos + dir * radius;
            const float gravityPower = gameGetGravityWellPowerAtRadius(well, radius);
            measures.push_back({point, radius, gravityPower});
        }

        sf::CircleShape gravityCircleShape;
        gravityCircleShape.setPosition(well.pos);

        for (const Measure& measure : measures)
        {
            gravityCircleShape.setRadius(measure.radius);
            gravityCircleShape.setOrigin(Vec2{gravityCircleShape.getRadius(), gravityCircleShape.getRadius()});

            sf::Uint8 grayscale = static_cast<sf::Uint8>(measure.gravityPower / well.maxPower * 255.f);
            gravityCircleShape.setFillColor(sf::Color{grayscale, grayscale, grayscale});

            window.draw(gravityCircleShape);
        }

        sf::CircleShape pointShape{5.f};
        pointShape.setOrigin(Vec2{pointShape.getRadius(), pointShape.getRadius()});
        pointShape.setFillColor(sf::Color::Magenta);

        sf::Text gravityPowerText{};
        gravityPowerText.setFont(font);
        gravityPowerText.setCharacterSize(20);
        gravityPowerText.setFillColor(sf::Color::Magenta);

        for (const Measure& measure : measures)
        {
            pointShape.setPosition(measure.point);
            window.draw(pointShape);

            gravityPowerText.setString(std::to_string(measure.gravityPower));
            gravityPowerText.setPosition(measure.point + Vec2{10.f, -10.f});
            window.draw(gravityPowerText);
        }

        sf::CircleShape dragCircleShape{well.dragRadius};
        dragCircleShape.setOrigin(Vec2{dragCircleShape.getRadius(), dragCircleShape.getRadius()});
        dragCircleShape.setPosition(well.pos);
        dragCircleShape.setFillColor(sf::Color::Transparent);
        dragCircleShape.setOutlineColor(sf::Color::Magenta);
        dragCircleShape.setOutlineThickness(5.f);
        window.draw(dragCircleShape);
    }

    for (const Ship& ship : world.ships)
    {
        {
            // ship collision body
            sf::CircleShape shipCollisionShape{world.settings.shipCollisionRadius};
            shipCollisionShape.setOrigin(Vec2{shipCollisionShape.getRadius(), shipCollisionShape.getRadius()});
            shipCollisionShape.setFillColor(ship.isDead ? sf::Color::Blue : sf::Color::Green);
            shipCollisionShape.setPosition(ship.pos);
            window.draw(shipCollisionShape);
        }

        // velocity
        drawThickLine(ship.pos, ship.pos + ship.velocity, 5.f, sf::Color::Yellow, window);

        // gravity
        for (const GravityWell& well : world.gravityWells)
        {
            Vec2 gravityPower = gameGetGravityWellVectorAtPoint(well, ship.pos);
            drawThickLine(ship.pos, ship.pos + gravityPower, 5.f, sf::Color::Magenta, window);
        }

        // input acceleration
        if (ship.input.accelerate)
        {
            Vec2 accel = vec2RotationToDir(ship.rotation) * world.settings.shipAcceleration;
            drawThickLine(ship.pos, ship.pos + accel, 5.f, sf::Color::Green, window);
        }

        {
            // ship direction
            sf::RectangleShape rectShape{Vec2{world.settings.shipCollisionRadius, 5.f}};
            rectShape.setPosition(ship.pos);
            rectShape.setOrigin(Vec2{0.f, rectShape.getSize().y / 2.f});
            rectShape.rotate(radToDeg(ship.rotation));
            rectShape.setFillColor(sf::Color::Black);
            window.draw(rectShape);
        }
    }

    {
        // projectiles
        sf::CircleShape projectileShape{5.f};
        projectileShape.setOrigin(Vec2{projectileShape.getRadius(), projectileShape.getRadius()});
        projectileShape.setFillColor(sf::Color::Red);

        for (const Projectile& projectile : world.projectiles)
        {
            projectileShape.setPosition(projectile.pos);
            window.draw(projectileShape);
        }
    }
}

GameWorld createWorld(const Vec2 size)
{
    GameWorld world;
    world.size = size;

    Ship ship1;
    ship1.pos = world.size / 2.f - size / 4.f;
    ship1.rotation = TAU / 8.f * 5.f;
    world.ships.push_back(ship1);

    Ship ship2;
    ship2.pos = world.size / 2.f + size / 4.f;
    ship2.rotation = TAU / 8.f;
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
    world.settings.shipSteeringSpeed = TAU / 2.f;
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
    sf::Text text("Hello SFML", font, 50);

    GameWorld world = createWorld(Vec2{window.getSize()});

    sf::RectangleShape shipShape;
    {
        const Vec2 size{35, 35};
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
    shipCollisionShape.setOrigin(Vec2{shipCollisionShape.getRadius(), shipCollisionShape.getRadius()});
    shipCollisionShape.setFillColor(sf::Color::Red);

    PlayerKeymap player1Keymap;
    player1Keymap.left = sf::Keyboard::A;
    player1Keymap.right = sf::Keyboard::D;
    player1Keymap.shoot = sf::Keyboard::W;
    player1Keymap.accelerate = sf::Keyboard::S;
    player1Keymap.accelerateImpulse = sf::Keyboard::LShift;

    PlayerKeymap player2Keymap;
    player2Keymap.left = sf::Keyboard::J;
    player2Keymap.right = sf::Keyboard::L;
    player2Keymap.shoot = sf::Keyboard::I;
    player2Keymap.accelerate = sf::Keyboard::K;
    player2Keymap.accelerateImpulse = sf::Keyboard::RShift;

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

        // input
        {
            if (world.ships.size() > 0)
            {
                world.ships[0].input = readPlayerInput(player1Keymap);
            }
            if (world.ships.size() > 1)
            {
                world.ships[1].input = readPlayerInput(player2Keymap);
            }
        }

        // update
        gameUpdate(world, dt);

        // render
        {
            window.clear();

            if (isDebugRender)
            {
                renderGameDebugView(world, window, font);
            }
            else
            {
                sf::CircleShape gravityWellShape{10.f};
                gravityWellShape.setOrigin(Vec2{gravityWellShape.getRadius(), gravityWellShape.getRadius()});
                gravityWellShape.setFillColor(sf::Color::Cyan);
                gravityWellShape.setPosition(world.size / 2.f);
                window.draw(gravityWellShape);


                bool kek = false;
                for (const Ship& ship : world.ships)
                {
                    if (kek)
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

                    // shipCollisionShape.setPosition(ship.pos);
                    // window.draw(shipCollisionShape);

                    kek = true;
                }

                for (const Projectile& projectile : world.projectiles)
                {
                    renderObj(projectile.pos, projectile.rotation, world.size, projectileShape, window);
                }
            }

            window.display();
        }
    }

    return EXIT_SUCCESS;
}
