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

GameVisualWorld createVisualWorld(const GameWorld& logicWorld)
{
    GameVisualWorld visualWorld;

    ParticlesEmitter& shipThrustEmitter = visualWorld.shipThrustEmitter;
    shipThrustEmitter.particlesPerSec = 30.f;
    shipThrustEmitter.angleRange = FloatRange{-10.f, 10.f};
    shipThrustEmitter.speedRange = FloatRange{180.f, 220.f};
    shipThrustEmitter.lifetimeRange = FloatRange{0.4f, 0.6f};
    shipThrustEmitter.startRadiusRange = FloatRange{7.f, 12.f};
    shipThrustEmitter.finishRadiusRange = FloatRange{1.f, 2.f};
    shipThrustEmitter.startColorRange = ColorRange{sf::Color{255, 0, 0, 150}, sf::Color{255, 150, 0, 150}};
    shipThrustEmitter.finishColorRange = ColorRange{sf::Color{0, 0, 0, 0}, sf::Color{25, 25, 0, 150}};

    visualWorld.shipsDead.resize(logicWorld.ships.size(), false);
    
    return visualWorld;
}

void startingStateSfEventHandler(AppStateStarting& startingState, const std::vector<Player>& players,
                                 const sf::Event& event)
{
    if (event.type != sf::Event::KeyReleased)
    {
        return;
    }

    for (size_t i = 0; i < players.size(); ++i)
    {
        const Player& player = players[i];
        forEachKeyInKeymap(player.keymap, [&event, &startingState, i](const sf::Keyboard::Key key)
        {
            if (event.key.code == key)
            {
                startingState.playersReady[i] = true;
            }
        });
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
        visualWorld = createVisualWorld(world);
    };

    initWorlds();

    std::vector<Player> players{
        {
            PlayerKeymap{
                sf::Keyboard::A, sf::Keyboard::D, sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::LShift
            },
            "WASD"
        },
        {
            PlayerKeymap{
                sf::Keyboard::J, sf::Keyboard::L, sf::Keyboard::I, sf::Keyboard::K, sf::Keyboard::RShift
            },
            "IJKL"
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
                world.ships[i].input = readPlayerInput(players[i].keymap);
            }

            std::optional<GameResult> simResult = gameSimulate(world, dt);
            gameVisualSimulate(visualWorld, world, dt);

            if (simResult.has_value())
            {
                if (!simResult->isTie())
                {
                    players[simResult->victoriousPlayerIndex].score++;
                }

                appState = AppStateGameOver{simResult.value(), 15.f};
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
            const float slowMotionMultiplier = floatLerp(0.2f, 1.f,std::clamp(gameOverState->timeInState / timeInSlowMotion, 0.f, 1.f)); 
            const float slowMotionDt = slowMotionMultiplier * dt;

            gameSimulate(world, slowMotionDt);
            gameVisualSimulate(visualWorld, world, slowMotionDt);

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
        window.clear();

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
