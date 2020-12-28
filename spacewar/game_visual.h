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

struct ParticleEmitSettings
{
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
    ParticleEmitSettings settings;
    
    float particlesPerSec = 0.f;
    float timer = 0.f;
};

struct ParticleBurstEmitter
{
    ParticleEmitSettings settings{};
    
    int particlesCount = 0;
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
    std::vector<DeadShipPiece> deadShipPieces;
    ParticleEmitter shipThrustEmitter;
    ParticleEmitter projectileTrailEmitter;
    ParticleBurstEmitter shipThrustBurstEmitSettings;
};

void gameVisualSimulate(GameVisualWorld& visualWorld, const GameWorld& logicWorld, const GameEvents& gameEvents, float dt);
