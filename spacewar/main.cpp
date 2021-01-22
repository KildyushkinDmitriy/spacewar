#include "game_logic.h"
#include "game_visual.h"
#include "player.h"
#include "draw_game.h"
#include "draw_ui.h"
#include "app_state.h"
#include "game_entities.h"

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

void update(entt::registry& registry, const float dt, const Vec2 worldSize)
{
    gravityWellSystem(registry, dt);
    rotateByInputSystem(registry);
    accelerateByInputSystem(registry, dt);
    accelerateImpulseSystem(registry, dt);
    applyRotationSpeedSystem(registry, dt);
    applyVelocitySystem(registry, dt);
    wrapPositionAroundWorldSystem(registry, worldSize);
    shootingSystem(registry, dt);
    projectileMoveSystem(registry, dt);
    circleVsCircleCollisionSystem(registry);
    teleportSystem(registry);

    spawnDeadShipPiecesOnCollisionSystem(registry);
    enableParticleEmitterByAccelerateInputSystem(registry);
    emitParticlesOnAccelerateImpulseSystem(registry);
    particleEmitterSystem(registry, dt);

    accelerateImpulseAppliedOneshotComponentClearSystem(registry);

    destroyByCollisionSystem(registry);
    destroyTimerSystem(registry, dt);
}


int main()
{
    runTests();

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window{sf::VideoMode{1000, 1000}, "Spacewar!", sf::Style::Default, settings};

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

    const auto initWorlds = [&world, &window]()
    {
        world = createWorld(Vec2{window.getSize()});
    };

    initWorlds();

    entt::registry registry;

    const auto shipEntity1 = createShipEntity(registry, world.size / 2.f - world.size / 4.f, 180.f + 45.f, sf::Color::Cyan, 0);
    const auto shipEntity2 = createShipEntity(registry, world.size / 2.f + world.size / 4.f, 45.f, sf::Color::White, 1);

    createGravityWellEntity(registry, world.size);
    createStarEntities(registry, world.size);

    std::vector<Player> players{
        {
            PlayerKeymap{
                sf::Keyboard::A, sf::Keyboard::D, sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::LShift
            },
            "WASD",
            sf::Keyboard::Q,
        },
        {
            PlayerKeymap{
                sf::Keyboard::J, sf::Keyboard::L, sf::Keyboard::I, sf::Keyboard::K, sf::Keyboard::RShift
            },
            "IJKL",
            sf::Keyboard::O
        }
    };

    players[0].shipEntity = shipEntity1;
    players[1].shipEntity = shipEntity2;

    AppState appState = AppStateGame{};
    // std::get<AppStateStarting>(appState).playersReady.resize(players.size(), false);

    bool isDebugRender = false;

    float time = 0.f;

    sf::Clock timer;
    while (window.isOpen())
    {
        const float dt = std::clamp(timer.restart().asSeconds(), 0.f, 0.1f);
        time += dt;

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

                if (registry.valid(player.shipEntity))
                {
                    auto input = readPlayerInput(player.keymap);
                    registry.get<AccelerateByInputComponent>(player.shipEntity).accelerateInput = input.thrust;
                    registry.get<RotateByInputComponent>(player.shipEntity).input = input.rotate;
                    registry.get<ShootingComponent>(player.shipEntity).input = input.shoot;
                    registry.get<AccelerateImpulseByInputComponent>(player.shipEntity).input = input.thrustBurst;
                }
            }

            update(registry, dt, world.size);

            // const GameEvents gameEvents = gameSimulate(world, dt);

            const GameEvents gameEvents{};

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
            const bool everyoneReady = std::all_of(readyVec.begin(), readyVec.end(), [](const bool ready)
            {
                return ready;
            });
            if (everyoneReady)
            {
                initWorlds();
                appState = AppStateGame{};
            }
        }

        // draw
        window.clear(sf::Color{5, 10, 30, 255});
        if (std::holds_alternative<AppStateGame>(appState) || std::holds_alternative<AppStateGameOver>(appState))
        {
            if (isDebugRender)
            {
                // drawGameDebug(world, window, font);
            }
            else
            {
                drawGame(window, shipTexture, registry, world.size, time);
            }
        }

        if (const auto* gameOverState = std::get_if<AppStateGameOver>(&appState))
        {
            drawGameOverUi(*gameOverState, players, window, font);
        }
        else if (const auto* startingState = std::get_if<AppStateStarting>(&appState))
        {
            drawStartingUi(*startingState, players, window, font);
        }

        window.display();
    }

    return EXIT_SUCCESS;
}
