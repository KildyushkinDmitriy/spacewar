﻿#pragma once
#include <variant>

struct AppStateStarting
{
    float timeInState = 0.f;
    std::vector<bool> playersReady;
};

struct AppStateGame
{
};

struct AppStateGameOver
{
    GameResult gameResult{};
    float timeWhenRestart = 0.f;
    float timeInState = 0.f;
};

using AppState = std::variant<AppStateStarting, AppStateGame, AppStateGameOver>;
