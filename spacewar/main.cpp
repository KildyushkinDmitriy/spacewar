#include "game_logic.h"
#include "player.h"
#include "app_state.h"
#include "game_frame.h"

#include <SFML/Graphics.hpp>
#include <memory>

void runTests();

int main()
{
    runTests();

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window{sf::VideoMode{1000, 1000}, "Spacewar!", sf::Style::Default, settings};

    AppPersistent appPersistentData{};

    if (!appPersistentData.shipTexture.loadFromFile("images/ship.png"))
    {
        return EXIT_FAILURE;
    }

    if (!appPersistentData.font.loadFromFile("fonts/arial.ttf"))
    {
        return EXIT_FAILURE;
    }
    
    appPersistentData.worldSize = Vec2{window.getSize()};

    appPersistentData.players = std::vector<Player>{
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

    appPersistentData.appStatePtr = std::make_unique<AppStateStarting>(appPersistentData.players.size());

    sf::Clock timer;
    while (window.isOpen())
    {
        const float dt = std::clamp(timer.restart().asSeconds(), 0.f, 0.1f);
        appPersistentData.time += dt;
        appPersistentData.appStatePtr->timeInState += dt;

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            appPersistentData.appStatePtr->processSfmlEvent(appPersistentData, event);
        }

        appPersistentData.appStatePtr->updateFrame(appPersistentData, dt);

        window.clear(sf::Color{5, 10, 30, 255});
        appPersistentData.appStatePtr->drawFrame(appPersistentData, window);
        window.display();
    }

    return EXIT_SUCCESS;
}
