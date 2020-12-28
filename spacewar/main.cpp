#include "game_logic.h"
#include "game_visual.h"
#include "player.h"
#include "render.h"

#include <SFML/Graphics.hpp>
#include <variant>
#include <cassert>

void runTests();

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

    world.settings.shipThrustAcceleration = 25.f;
    world.settings.shipThrustBurstImpulse = 75.f;
    world.settings.shipThrustBurstImpulseCooldown = 3.f;
    world.settings.shipRotationSpeed = 180.f;
    world.settings.shootCooldown = 1.f;
    world.settings.projectileSpeed = 200.f;
    world.settings.projectileLifetime = 5.f;
    world.settings.shipCollisionRadius = 15.f;
    world.settings.muzzleExtraOffset = 25.f;
    world.settings.gravityWellDragCoefficient = 0.3f;

    return world;
}

GameVisualWorld createVisualWorld(const Vec2 worldSize)
{
    GameVisualWorld visualWorld;

    visualWorld.shipThrustEmitter.particlesPerSec = 30.f;
    ParticleEmitSettings& shipThrustEmitter = visualWorld.shipThrustEmitter.settings;
    shipThrustEmitter.angleRange = FloatRange{-10.f, 10.f};
    shipThrustEmitter.speedRange = FloatRange{180.f, 220.f};
    shipThrustEmitter.lifetimeRange = FloatRange{0.4f, 0.6f};
    shipThrustEmitter.startRadiusRange = FloatRange{7.f, 12.f};
    shipThrustEmitter.finishRadiusRange = FloatRange{1.f, 2.f};
    shipThrustEmitter.startColorRange = ColorRange{sf::Color{255, 0, 0, 150}, sf::Color{255, 150, 0, 150}};
    shipThrustEmitter.finishColorRange = ColorRange{sf::Color{0, 0, 0, 0}, sf::Color{25, 25, 0, 150}};

    visualWorld.shipThrustBurstEmitSettings.particlesCount = 50;
    ParticleEmitSettings& shipThrustBurstEmitter = visualWorld.shipThrustBurstEmitSettings.settings;
    shipThrustBurstEmitter.angleRange = FloatRange{-30.f, 30.f};
    shipThrustBurstEmitter.speedRange = FloatRange{150.f, 300.f};
    shipThrustBurstEmitter.lifetimeRange = FloatRange{0.2f, 0.4f};
    shipThrustBurstEmitter.startRadiusRange = FloatRange{9.f, 15.f};
    shipThrustBurstEmitter.finishRadiusRange = FloatRange{2.f, 3.f};
    shipThrustBurstEmitter.startColorRange = ColorRange{sf::Color{255, 100, 0, 150}, sf::Color{255, 150, 0, 150}};
    shipThrustBurstEmitter.finishColorRange = ColorRange{sf::Color{0, 0, 0, 0}, sf::Color{25, 25, 0, 150}};

    visualWorld.projectileTrailEmitter.particlesPerSec = 30.f;
    ParticleEmitSettings& projectileTrailEmitter = visualWorld.projectileTrailEmitter.settings;
    projectileTrailEmitter.angleRange = FloatRange{-5.f, 5.f};
    projectileTrailEmitter.speedRange = FloatRange{15.f, 17.f};
    projectileTrailEmitter.lifetimeRange = FloatRange{0.25f, 0.3f};
    projectileTrailEmitter.startRadiusRange = FloatRange{4.f, 5.f};
    projectileTrailEmitter.finishRadiusRange = FloatRange{1.f, 1.3f};
    projectileTrailEmitter.startColorRange = ColorRange{sf::Color{100, 0, 0, 150}, sf::Color{100, 0, 0, 150}};
    projectileTrailEmitter.finishColorRange = ColorRange{sf::Color{0, 0, 0, 0}, sf::Color{0, 0, 0, 150}};

    for (int i = 0; i < 50; ++i)
    {
        Star star;
        star.pos = Vec2{randomFloatRange(0, worldSize.x), randomFloatRange(0, worldSize.y)};
        star.radius = randomFloatRange(0.5f, 2.f);
        star.brightnessRange = FloatRange{randomFloatRange(0.4f, 0.7f), randomFloatRange(0.7f, 1.f)};
        star.periodsPerSec = randomFloatRange(0.1f, 0.5f);
        visualWorld.stars.push_back(star);
    }

    return visualWorld;
}

void startingStateSfEventHandler(AppStateStarting& startingState, std::vector<Player>& players, const sf::Event& event)
{
    if (event.type != sf::Event::KeyReleased)
    {
        return;
    }

    for (size_t i = 0; i < players.size(); ++i)
    {
        Player& player = players[i];
        forEachKeyInKeymap(player.keymap, [&event, &startingState, i](const sf::Keyboard::Key key)
        {
            if (event.key.code == key)
            {
                startingState.playersReady[i] = true;
            }
        });

        if (event.key.code == player.makeAiKey)
        {
            player.isAi = true;
            startingState.playersReady[i] = true;
        }
    }
}

int main()
{
    runTests();

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window{sf::VideoMode{1000, 1000}, "Spacewars!", sf::Style::Default, settings};

    sf::Texture shipTexture;
    if (!shipTexture.loadFromFile("images/ship.png"))
    {
        return EXIT_FAILURE;
    }

    sf::Font font;
    if (!font.loadFromFile("fonts/arial.ttf"))
    {
        return EXIT_FAILURE;
    }

    GameWorld world;
    GameVisualWorld visualWorld;

    const auto initWorlds = [&world, &visualWorld, &window]()
    {
        world = createWorld(Vec2{window.getSize()});
        visualWorld = createVisualWorld(world.size);
    };

    initWorlds();

    std::vector<Player> players{
        {
            PlayerKeymap{
                sf::Keyboard::A, sf::Keyboard::D, sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::LShift
            },
            "WASD",
            sf::Keyboard::Q
        },
        {
            PlayerKeymap{
                sf::Keyboard::J, sf::Keyboard::L, sf::Keyboard::I, sf::Keyboard::K, sf::Keyboard::RShift
            },
            "IJKL",
            sf::Keyboard::O
        }
    };

    AppState appState = AppStateStarting{};
    std::get<AppStateStarting>(appState).playersReady.resize(players.size(), false);

    bool isDebugRender = false;

    sf::Clock timer;
    while (window.isOpen())
    {
        const float dt = std::clamp(timer.restart().asSeconds(), 0.f, 0.1f);

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tilde)
            {
                isDebugRender = !isDebugRender;
            }

            if (auto* startingState = std::get_if<AppStateStarting>(&appState))
            {
                startingStateSfEventHandler(*startingState, players, event);
            }
        }

        assert(world.ships.size() == players.size());

        // update
        if (std::holds_alternative<AppStateGame>(appState))
        {
            // player input
            for (size_t i = 0; i < players.size(); ++i)
            {
                const Player& player = players[i];

                world.ships[i].input = player.isAi
                                           ? aiGenerateInput(world, i, (i + 1) % players.size())
                                           : readPlayerInput(player.keymap);
            }

            const GameEvents gameEvents = gameSimulate(world, dt);
            gameVisualSimulate(visualWorld, world, gameEvents, dt);

            if (gameEvents.result.has_value())
            {
                if (!gameEvents.result->isTie())
                {
                    players[gameEvents.result->victoriousPlayerIndex].score++;
                }

                appState = AppStateGameOver{gameEvents.result.value(), 15.f};
            }
        }
        else if (auto* gameOverState = std::get_if<AppStateGameOver>(&appState))
        {
            gameOverState->timeInState += dt;

            for (Ship& ship : world.ships)
            {
                ship.input = {};
            }

            constexpr float timeInSlowMotion = 2.f;
            const float t = std::clamp(gameOverState->timeInState / timeInSlowMotion, 0.f, 1.f);
            const float slowMotionMultiplier = floatLerp(0.2f, 1.f, t);
            const float slowMotionDt = slowMotionMultiplier * dt;

            const GameEvents gameEvents = gameSimulate(world, slowMotionDt);
            gameVisualSimulate(visualWorld, world, gameEvents, slowMotionDt);

            const bool restartButtonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

            if (restartButtonPressed || gameOverState->timeInState > gameOverState->timeWhenRestart)
            {
                initWorlds();
                appState = AppStateGame{};
            }
        }
        else if (auto* startingState = std::get_if<AppStateStarting>(&appState))
        {
            startingState->timeInState += dt;

            std::vector<bool>& readyVec = startingState->playersReady;

            const bool allReady = std::all_of(readyVec.begin(), readyVec.end(), [](const bool ready)
            {
                return ready;
            });

            if (allReady)
            {
                initWorlds();
                appState = AppStateGame{};
            }
        }

        // render
        window.clear(sf::Color{5, 10, 30, 255});;
        if (std::holds_alternative<AppStateGame>(appState) || std::holds_alternative<AppStateGameOver>(appState))
        {
            if (isDebugRender)
            {
                renderGameDebug(world, window, font);
            }
            else
            {
                renderGame(world, visualWorld, window, shipTexture);
            }
        }

        if (const auto* gameOverState = std::get_if<AppStateGameOver>(&appState))
        {
            renderGameOverUi(*gameOverState, players, window, font);
        }
        else if (const auto* startingState = std::get_if<AppStateStarting>(&appState))
        {
            renderStartingUi(*startingState, players, window, font);
        }

        window.display();
    }

    return EXIT_SUCCESS;
}
