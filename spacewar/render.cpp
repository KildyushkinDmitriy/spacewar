#include "game_logic.h"

#include <SFML/Graphics.hpp>

#include "app_state.h"

static void drawThickLine(const Vec2 a, const Vec2 b, const float thickness, const sf::Color color,
                          sf::RenderWindow& window)
{
    const Vec2 diff = b - a;
    const float angleRad = std::atan2f(diff.y, diff.x);

    sf::RectangleShape rectShape{Vec2{vec2Length(diff), thickness}};
    rectShape.setPosition(a);
    rectShape.setOrigin(Vec2{0.f, thickness / 2.f});
    rectShape.rotate(radToDeg(angleRad));
    rectShape.setFillColor(color);
    window.draw(rectShape);
}

static void renderObj(const Vec2 pos, const float rotation, const Vec2& worldSize, sf::Shape& renderShape,
                      sf::RenderWindow& window)
{
    // sprite is pointing upwards, but with zero rotation it must be pointing right, so offset the rotation
    renderShape.setRotation(rotation + 90.f);

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

void renderGameOverUi(const AppState::GameOver& gameOverState, sf::RenderWindow& window, sf::Font& font)
{
    const GameResult& gameResult = gameOverState.gameResult;
    sf::String gameResultString;

    if (gameResult.isTie())
    {
        gameResultString = "Tie!";
    }
    else if (gameResult.victoriousPlayerIndex == 0)
    {
        gameResultString = "WASD Player wins!";
    }
    else if (gameResult.victoriousPlayerIndex == 1)
    {
        gameResultString = "IJKL Player wins!";
    }
    else
    {
        gameResultString = "Player " + std::to_string(gameResult.victoriousPlayerIndex + 1) + " wins!";
    }

    sf::Text textRender{};
    textRender.setCharacterSize(40);
    textRender.setFont(font);
    textRender.setOutlineThickness(2.f);
    textRender.setOutlineColor(sf::Color::Black);

    textRender.setString(gameResultString);
    {
        const sf::FloatRect localBounds = textRender.getLocalBounds();
        textRender.setPosition(Vec2{window.getSize()} / 2.f - Vec2{localBounds.width / 2.f, 150.f});
    }
    window.draw(textRender);

    const int restartTimeLeftInt = static_cast<int>(gameOverState.restartTimeLeft + 1.f);
    textRender.setString("Next round in " + std::to_string(restartTimeLeftInt));
    {
        const sf::FloatRect localBounds = textRender.getLocalBounds();
        textRender.setPosition(Vec2{window.getSize()} / 2.f - Vec2{localBounds.width / 2.f, -150.f});
    }
    window.draw(textRender);
}

void renderGameDebug(const GameWorld& world, sf::RenderWindow& window, sf::Font& font)
{
    // gravitation field visualize
    for (const GravityWell& well : world.gravityWells)
    {
        const int gradationsCount = 20;
        const float radiusIncrement = well.maxRadius / (gradationsCount - 1);
        const Vec2 dir = vec2RotationToDir(180.f + 45.f);

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

        sf::Text gravityPowerText;
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
            rectShape.rotate(ship.rotation);
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
