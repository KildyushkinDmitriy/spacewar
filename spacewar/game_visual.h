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

struct ParticleEmitterSettings
{
    float particlesPerSec = 0.f;
    int particlesPerSpawn = 1;

    FloatRange angleRange{};
    FloatRange speedRange{};
    FloatRange lifetimeRange{};

    FloatRange startRadiusRange{};
    FloatRange finishRadiusRange{};

    ColorRange startColorRange;
    ColorRange finishColorRange;
};

struct ParticleEmitter
{
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

struct Star
{
    Vec2 pos{};
    float radius = 0.f;

    FloatRange brightnessRange{};
    float periodsPerSec = 0.f;
};

struct GameVisualWorld
{
    ParticleEmitterSettings shipThrustEmitterSettings{};
    ParticleEmitterSettings projectileTrailEmitterSettings{};
    ParticleEmitterSettings shipThrustBurstEmitSettings{};

    std::vector<ParticleEmitter> shipThrustParticleEmitters{};
    std::vector<ParticleEmitter> projectileParticleEmitters{};
    std::vector<Particle> particles{};

    std::vector<DeadShipPiece> deadShipPieces{};
    std::vector<Star> stars{};
};

void gameVisualSimulate(GameVisualWorld& visualWorld, const GameWorld& logicWorld, const GameEvents& gameEvents,
                        float dt);
