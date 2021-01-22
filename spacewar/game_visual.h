#pragma once

#include "game_logic.h"

#include <vector>

struct ParticleEmitterSettings
{
    float emitOffset = 0.f;
    float emitAngleOffset = 0.f;

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

struct ParticleComponent
{
    float totalLifetime = 0;

    sf::Color startColor{};
    sf::Color finishColor{};

    float startRadius = 0.f;
    float finishRadius = 0.f;
};

struct ParticleEmitterComponent
{
    ParticleEmitterSettings settings{};

    bool isEnabled = false;
    float timer = 0.f;
};

struct DrawUsingShipTextureComponent
{
    Vec2 size{};
    sf::Color color{};
};

struct EnableParticleEmitterByAccelerateInputComponent
{
};

struct ParticleEmitterOnAccelerateImpulseComponent
{
    ParticleEmitterSettings settings{};
};

struct DeadShipPieceComponent
{
    int pieceIndex = 0;
};

struct StarComponent
{
    float radius = 0.f;
    FloatRange brightnessRange{};
    float brightnessPeriodsPerSec = 0.f;
};

void particleEmitterSystem(entt::registry& registry, float dt);
void enableParticleEmitterByAccelerateInputSystem(entt::registry& registry);
void emitParticlesOnAccelerateImpulseSystem(entt::registry& registry);
void spawnDeadShipPiecesOnCollisionSystem(entt::registry& registry);
