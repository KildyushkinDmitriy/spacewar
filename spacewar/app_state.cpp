#include "app_state.h"
#include "draw_game.h"
#include "draw_ui.h"
#include "game_entities.h"
#include "game_frame.h"

void AppStateBase::trySwitchDbgDrawMode(AppPersistent& app, const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tilde)
    {
        app.isDebugRender = !app.isDebugRender;
    }
}

AppStateStarting::AppStateStarting(const int playersCount)
{
    m_playersReady.resize(playersCount, false);
}

void AppStateStarting::processSfmlEvent(AppPersistent& app, const sf::Event& event)
{
    if (event.type != sf::Event::KeyReleased)
    {
        return;
    }

    for (size_t i = 0; i < app.players.size(); ++i)
    {
        Player& player = app.players[i];
        forEachKeyInKeymap(player.keymap, [&event, this, i](const sf::Keyboard::Key key)
        {
            if (event.key.code == key)
            {
                m_playersReady[i] = true;
            }
        });

        if (event.key.code == player.makeAiKey)
        {
            player.isAi = true;
            m_playersReady[i] = true;
        }
    }
}

void AppStateStarting::updateFrame(AppPersistent& app, float dt)
{
    const bool everyoneReady = std::all_of(m_playersReady.begin(), m_playersReady.end(), [](const bool ready)
    {
        return ready;
    });
    if (everyoneReady)
    {
        recreateGameWorld(app.registry, app.players, app.worldSize);
        app.appStatePtr = std::make_unique<AppStateGame>();
    }
}

void AppStateStarting::drawFrame(const AppPersistent& app, sf::RenderWindow& window)
{
    drawStartingUi(m_playersReady, app.players, window, app.font, timeInState);
}

void AppStateGame::processSfmlEvent(AppPersistent& app, const sf::Event& event)
{
    trySwitchDbgDrawMode(app, event);
}

void AppStateGame::updateFrame(AppPersistent& app, const float dt)
{
    entt::registry& registry = app.registry;

    for (const Player& player : app.players)
    {
        if (registry.valid(player.shipEntity))
        {
            const ShipInput input = player.isAi ? aiGenerateInput(registry, player.shipEntity) : readPlayerInput(player.keymap);
            registry.get<AccelerateByInputComponent>(player.shipEntity).input = input.thrust;
            registry.get<RotateByInputComponent>(player.shipEntity).input = input.rotate;
            registry.get<ShootingComponent>(player.shipEntity).input = input.shoot;
            registry.get<AccelerateImpulseByInputComponent>(player.shipEntity).input = input.thrustBurst;
        }
    }

    gameFrameUpdate(registry, dt, app.worldSize);

    std::optional<GameResult> optGameResult = tryGetGameResult(registry, app.players.size());

    if (optGameResult.has_value())
    {
        if (!optGameResult->isTie())
        {
            app.players[optGameResult->victoriousPlayerIndex].score++;
        }

        app.appStatePtr = std::make_unique<AppStateGameOver>(optGameResult.value());
    }
}

void AppStateGame::drawFrame(const AppPersistent& app, sf::RenderWindow& window)
{
    if (app.isDebugRender)
    {
        drawGameDebug(app.registry, window, app.font);
    }
    else
    {
        drawGame(window, app.shipTexture, app.registry, app.worldSize, app.time);
    }
}

AppStateGameOver::AppStateGameOver(const GameResult& gameResult): m_gameResult(gameResult)
{
}

void AppStateGameOver::processSfmlEvent(AppPersistent& app, const sf::Event& event)
{
    trySwitchDbgDrawMode(app, event);
}

void AppStateGameOver::updateFrame(AppPersistent& app, const float dt)
{
    constexpr float timeInSlowMotion = 2.f;
    const float t = std::clamp(timeInState / timeInSlowMotion, 0.f, 1.f);
    const float slowMotionMultiplier = floatLerp(0.2f, 1.f, t);
    const float slowMotionDt = slowMotionMultiplier * dt;

    gameFrameUpdate(app.registry, slowMotionDt, app.worldSize);

    const bool restartButtonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    if (restartButtonPressed || timeInState > TIME_WHEN_RESTART)
    {
        recreateGameWorld(app.registry, app.players, app.worldSize);
        app.appStatePtr = std::make_unique<AppStateGame>();
    }
}

void AppStateGameOver::drawFrame(const AppPersistent& app, sf::RenderWindow& window)
{
    drawGame(window, app.shipTexture, app.registry, app.worldSize, app.time);
    drawGameOverUi(m_gameResult, TIME_WHEN_RESTART, timeInState, app.players, window, app.font);
}
