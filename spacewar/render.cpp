#include "render.h"

#include <SFML/Graphics.hpp>

#include "game_visual.h"

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

static void positionTextWithCenterAlignment(sf::Text& textRender, const Vec2 pos)
{
    const sf::FloatRect localBounds = textRender.getLocalBounds();
    textRender.setPosition(pos - Vec2{localBounds.width / 2.f, localBounds.height / 2.f});
}

void renderGame(const GameWorld& world, const GameVisualWorld& visualWorld, sf::RenderWindow& window,
                const sf::Texture& shipTexture)
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

    sf::CircleShape particleShape;
    for (const Particle& particle : visualWorld.particles)
    {
        const float t = particle.lifetime / particle.totalLifetime;
        const float radius = floatLerp(particle.startRadius, particle.finishRadius, t);
        const sf::Color color = colorLerp(particle.startColor, particle.finishColor, t);

        particleShape.setRadius(radius);
        particleShape.setOrigin(Vec2{particleShape.getRadius(), particleShape.getRadius()});
        particleShape.setPosition(particle.pos);
        particleShape.setFillColor(color);
        window.draw(particleShape);
    }

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

void renderGameOverUi(
    const AppStateGameOver& gameOverState,
    const std::vector<Player>& players,
    sf::RenderWindow& window,
    const sf::Font& font
)
{
    const float time = gameOverState.timeInState;
    const Vec2 windowCenter = Vec2{window.getSize()} / 2.f;

    sf::Text textRender;
    textRender.setCharacterSize(40);
    textRender.setFont(font);
    textRender.setOutlineThickness(2.f);
    textRender.setOutlineColor(sf::Color::Black);

    float animationTime = 0.f;

    animationTime += 1.f;

    if (time > animationTime)
    {
        const GameResult& gameResult = gameOverState.gameResult;
        sf::String gameResultString;

        if (gameResult.isTie())
        {
            gameResultString = "Tie!";
        }
        else
        {
            gameResultString = players[gameResult.victoriousPlayerIndex].name + " player wins round!";
        }

        textRender.setString(gameResultString);
        positionTextWithCenterAlignment(textRender, windowCenter + Vec2{0.f, -150.f});
        window.draw(textRender);

        animationTime += 1.f;
    }

    if (players.size() > 0 && time > animationTime)
    {
        const Player& player = players[0];
        textRender.setString(player.name + " score");
        positionTextWithCenterAlignment(textRender, windowCenter + Vec2{-300.f, 0.f});
        window.draw(textRender);

        textRender.setString(std::to_string(player.score));
        positionTextWithCenterAlignment(textRender, windowCenter + Vec2{-300.f, 50.f});
        window.draw(textRender);
    }

    if (players.size() > 1 && time > animationTime)
    {
        const Player& player = players[1];
        textRender.setString(player.name + " score");
        positionTextWithCenterAlignment(textRender, windowCenter + Vec2{300.f, 0.f});
        window.draw(textRender);

        textRender.setString(std::to_string(player.score));
        positionTextWithCenterAlignment(textRender, windowCenter + Vec2{300.f, 50.f});
        window.draw(textRender);
    }

    animationTime += 1.f;

    if (time > animationTime)
    {
        const int restartTimeLeftInt = static_cast<int>(gameOverState.timeWhenRestart - gameOverState.timeInState + 1.f
        );
        textRender.setString("Next round in " + std::to_string(restartTimeLeftInt));
        positionTextWithCenterAlignment(textRender, windowCenter + Vec2{0.f, 200.f});
        window.draw(textRender);
        textRender.setString("or press Space");
        positionTextWithCenterAlignment(textRender, windowCenter + Vec2{0.f, 250.f});
        window.draw(textRender);
    }
}


void renderStartingUi(const AppStateStarting& startingState, const std::vector<Player>& players,
                      sf::RenderWindow& window, const sf::Font& font)
{
    const float time = startingState.timeInState;
    const Vec2 windowCenter = Vec2{window.getSize()} / 2.f;

    sf::Text textRender;
    textRender.setFont(font);
    textRender.setOutlineThickness(2.f);
    textRender.setOutlineColor(sf::Color::Black);

    float animationTime = 0.f;

    {
        constexpr float gameTitleAppearTime = 1.f;
        animationTime += gameTitleAppearTime;
        // game title
        static const std::string gameFullTitleStr = "SPACEWAR!";

        const int curTitleLength = static_cast<int>(time / (gameTitleAppearTime / gameFullTitleStr.size()));
        const std::string gameTitleStr = gameFullTitleStr.substr(0, curTitleLength);

        textRender.setCharacterSize(80);
        textRender.setString(gameTitleStr);
        positionTextWithCenterAlignment(textRender, windowCenter + Vec2{0, -300.f});
        window.draw(textRender);
    }

    textRender.setCharacterSize(40);

    animationTime += 1.f;

    constexpr float timeBetweenLinesAppear = 0.3f;
    constexpr float yIncrement = 50.f;
    Vec2 curPos;

    const auto drawTextLine = [&](const std::string str)
    {
        if (time > animationTime)
        {
            textRender.setString(str);
            textRender.setPosition(curPos);
            window.draw(textRender);
            curPos.y += yIncrement;
            animationTime += timeBetweenLinesAppear;
        }
    };

    if (players.size() > 0)
    {
        const Player& player = players[0];
        curPos = windowCenter + Vec2{-400.f, 0.f};

        drawTextLine(player.name + " Player:");
        drawTextLine("A,D - rotate");
        drawTextLine("W - shoot");
        drawTextLine("S - thrust");
        drawTextLine("Left Shift - burst");

        if (time > animationTime)
        {
            textRender.setString(startingState.playersReady[0] ? "Ready" : "Press any key");
            textRender.setPosition(windowCenter + Vec2{-400.f, 300.f});
            window.draw(textRender);
            animationTime += timeBetweenLinesAppear;
        }
    }

    if (players.size() > 1)
    {
        const Player& player = players[1];
        curPos = windowCenter + Vec2{100.f, 0.f};

        drawTextLine(player.name + " Player:");
        drawTextLine("J,L - rotate");
        drawTextLine("I - shoot");
        drawTextLine("K - thrust");
        drawTextLine("Right Shift - burst");

        if (time > animationTime)
        {
            textRender.setString(startingState.playersReady[1] ? "Ready" : "Press any key");
            textRender.setPosition(windowCenter + Vec2{100.f, 300.f});
            window.draw(textRender);
            animationTime += timeBetweenLinesAppear;
        }
    }
}

void renderGameDebug(const GameWorld& world, sf::RenderWindow& window, const sf::Font& font)
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
