#pragma once
#include <variant>

struct AppStateStarting
{
    float time = 0.f;
    std::vector<bool> playersPushedButtons;
};

struct AppStateGame
{
};

struct AppStateGameOver
{
    GameResult gameResult{};
    float restartTimeLeft = 0.f;
};

using AppState = std::variant<AppStateStarting, AppStateGame, AppStateGameOver>;
