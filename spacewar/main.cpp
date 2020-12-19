#include <algorithm>
#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "SFML window");

    sf::Texture texture;
    if (!texture.loadFromFile("images/image.png"))
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

    // window.setFramerateLimit(60);

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
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            sf::Vector2f pos = sprite.getPosition();
            pos.x += 50.f * dt;
            sprite.setPosition(pos);
        }

        window.clear();

        window.draw(sprite);
        window.draw(text);

        window.display();
    }
    
    return EXIT_SUCCESS;
}
