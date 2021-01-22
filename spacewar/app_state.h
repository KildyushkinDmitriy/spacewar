#pragma once

#include "player.h"

#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>

struct AppPersistent
{
    entt::registry registry{};
    Vec2 worldSize{};
    std::vector<Player> players{};

    sf::Texture shipTexture{};
    sf::Font font{};

    bool isDebugRender = false;
    float time = 0.f;

    std::unique_ptr<class AppStateBase> appStatePtr{};
};

class AppStateBase
{
public:
    float timeInState = 0.f;

    virtual ~AppStateBase() = default;

    virtual void processSfmlEvent(AppPersistent& app, const sf::Event& event) = 0;
    virtual void updateFrame(AppPersistent& app, float dt) = 0;
    virtual void drawFrame(const AppPersistent& app, sf::RenderWindow& window) = 0;

    static void trySwitchDbgDrawMode(AppPersistent& app, const sf::Event& event);
};

class AppStateStarting : public AppStateBase
{
public:
    explicit AppStateStarting(int playersCount);

    void processSfmlEvent(AppPersistent& app, const sf::Event& event) override;
    void updateFrame(AppPersistent& app, float dt) override;
    void drawFrame(const AppPersistent& app, sf::RenderWindow& window) override;
private:
    std::vector<bool> m_playersReady{};
};

class AppStateGame : public AppStateBase
{
public:
    virtual void processSfmlEvent(AppPersistent& app, const sf::Event& event) override;
    virtual void updateFrame(AppPersistent& app, float dt) override;
    virtual void drawFrame(const AppPersistent& app, sf::RenderWindow& window) override;
};

class AppStateGameOver : public AppStateBase
{
public:
    explicit AppStateGameOver(const GameResult& gameResult);

    virtual void processSfmlEvent(AppPersistent& app, const sf::Event& event) override;
    virtual void updateFrame(AppPersistent& app, float dt) override;
    virtual void drawFrame(const AppPersistent& app, sf::RenderWindow& window) override;

private:
    GameResult m_gameResult{};

    constexpr static float TIME_WHEN_RESTART = 15.f;
};
