#include "draw_ui.h"

static void positionTextWithCenterAlignment(sf::Text& textRender, const Vec2 pos)
{
    const sf::FloatRect localBounds = textRender.getLocalBounds();
    textRender.setPosition(pos - Vec2{localBounds.width / 2.f, localBounds.height / 2.f});
}

void drawGameOverUi(
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


void drawStartingUi(const AppStateStarting& startingState, const std::vector<Player>& players,
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

    animationTime += 0.5f;

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
            textRender.setString(startingState.playersReady[0] ? players[0].isAi ? "AI" : "Ready" : "Press any key");
            textRender.setPosition(windowCenter + Vec2{-400.f, 300.f});
            window.draw(textRender);

            if (!startingState.playersReady[0])
            {
                textRender.setString("or Q for AI");
                textRender.setPosition(windowCenter + Vec2{-400.f, 350.f});
                window.draw(textRender);
            }

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
            textRender.setString(startingState.playersReady[1] ? players[0].isAi ? "AI" : "Ready" : "Press any key");
            textRender.setPosition(windowCenter + Vec2{100.f, 300.f});
            window.draw(textRender);

            if (!startingState.playersReady[1])
            {
                textRender.setString("or O for AI");
                textRender.setPosition(windowCenter + Vec2{100.f, 350.f});
                window.draw(textRender);
            }

            animationTime += timeBetweenLinesAppear;
        }
    }

    if (time > animationTime)
    {
        textRender.setCharacterSize(30);
        textRender.setString("press ~ in game for debug view");
        positionTextWithCenterAlignment(textRender, windowCenter + Vec2{0.f, 475.f});
        window.draw(textRender);
    }
}
