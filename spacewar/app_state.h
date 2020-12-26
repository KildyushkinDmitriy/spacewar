#pragma once

namespace AppState
{
    struct Game
    {
    };

    struct GameOver
    {
        GameResult gameResult{};
        float restartTimeLeft = 0.f;
    };
}
