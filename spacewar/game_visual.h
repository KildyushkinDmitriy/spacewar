#pragma once

#include "game_logic.h"

#include <vector>

struct Particle
{
    // const:
    float totalLifetime = 0;

    sf::Color startColor{};
    sf::Color finishColor{};

    float startRadius = 0.f;
    float finishRadius = 0.f;

    // mut:
    Vec2 pos{};
    Vec2 velocity{};

    float lifetime = 0;
};

struct ParticlesEmitter
{
    // const:
    float particlesPerSec = 0.f;

    FloatRange angleRange{};
    FloatRange speedRange{};
    FloatRange lifetimeRange{};

    FloatRange startRadiusRange{};
    FloatRange finishRadiusRange{};

    ColorRange startColorRange;
    ColorRange finishColorRange;

    // mut:
    float timer = 0.f;
};

struct DeadShipPiece
{
    Vec2 pos{};
    Vec2 velocity{};

    float rotation = 0.f;
    float angularSpeed = 0.f;

    int shipIndex = 0;
    int pieceIndex = 0;
};

struct GameVisualWorld
{
    std::vector<Particle> particles;
    std::vector<bool> shipsDead; // used to track that visual sim handled ship death
    std::vector<DeadShipPiece> deadShipPieces;
    ParticlesEmitter shipThrustEmitter;
};

void gameVisualSimulate(GameVisualWorld& visualWorld, const GameWorld& logicWorld, float dt);
