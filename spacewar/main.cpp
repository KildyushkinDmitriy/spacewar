#include "game_logic.h"
#include "player.h"
#include "draw_game.h"
#include "draw_ui.h"
#include "app_state.h"
#include "game_entities.h"
#include "game_frame.h"

#include <SFML/Graphics.hpp>
#include <variant>
#include <cassert>

void runTests();

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

    const Vec2 worldSize = Vec2{window.getSize()};

    entt::registry registry;

    recreateGameWorld(registry, players, worldSize);

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

        // update
        if (std::holds_alternative<AppStateGame>(appState))
        {
            // player input
            for (size_t i = 0; i < players.size(); ++i)
            {
                const Player& player = players[i];
                // world.ships[i].input = player.isAi
                // ? aiGenerateInput(world, i, (i + 1) % players.size())
                // : readPlayerInput(player.keymap);

                if (registry.valid(player.shipEntity))
                {
                    auto input = readPlayerInput(player.keymap);
                    registry.get<AccelerateByInputComponent>(player.shipEntity).input = input.thrust;
                    registry.get<RotateByInputComponent>(player.shipEntity).input = input.rotate;
                    registry.get<ShootingComponent>(player.shipEntity).input = input.shoot;
                    registry.get<AccelerateImpulseByInputComponent>(player.shipEntity).input = input.thrustBurst;
                }
            }

            gameFrameUpdate(registry, dt, worldSize);

            std::optional<GameEventGameResult> optGameResult = tryGetGameResult(registry, players.size());

            if (optGameResult.has_value())
            {
                if (!optGameResult->isTie())
                {
                    players[optGameResult->victoriousPlayerIndex].score++;
                }

                appState = AppStateGameOver{optGameResult.value(), 15.f};
            }
        }
        else if (auto* gameOverState = std::get_if<AppStateGameOver>(&appState))
        {
            gameOverState->timeInState += dt;

            constexpr float timeInSlowMotion = 2.f;
            const float t = std::clamp(gameOverState->timeInState / timeInSlowMotion, 0.f, 1.f);
            const float slowMotionMultiplier = floatLerp(0.2f, 1.f, t);
            const float slowMotionDt = slowMotionMultiplier * dt;

            gameFrameUpdate(registry, slowMotionDt, worldSize);

            const bool restartButtonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
            if (restartButtonPressed || gameOverState->timeInState > gameOverState->timeWhenRestart)
            {
                recreateGameWorld(registry, players, worldSize);
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
                recreateGameWorld(registry, players, worldSize);
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
                drawGame(window, shipTexture, registry, worldSize, time);
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
