#include "game_visual.h"

static void simulateParticleEmitter(
    std::vector<Particle>& particles,
    ParticleEmitter& emitter,
    const Vec2 pos,
    const float baseAngle,
    const float dt
)
{
    const float timeBetweenParticles = 1.f / emitter.particlesPerSec;
    emitter.timer += dt;

    while (emitter.timer > timeBetweenParticles)
    {
        emitter.timer -= timeBetweenParticles;

        Particle particle;
        particle.pos = pos;

        const float angle = baseAngle + emitter.angleRange.getRandom();
        const Vec2 dir = vec2RotationToDir(angle);
        const float speed = emitter.speedRange.getRandom();

        particle.velocity = dir * speed;
        particle.totalLifetime = emitter.lifetimeRange.getRandom();

        particle.startRadius = emitter.startRadiusRange.getRandom();
        particle.finishRadius = emitter.finishRadiusRange.getRandom();

        particle.startColor = emitter.startColorRange.getRandom();
        particle.finishColor = emitter.finishColorRange.getRandom();

        particles.push_back(particle);
    }
}

void gameVisualSimulate(GameVisualWorld& visualWorld, const GameWorld& logicWorld, const GameEvents& gameEvents, const float dt)
{
    // ship thrust emitters
    for (const Ship& ship : logicWorld.ships)
    {
        if (ship.input.accelerate)
        {
            const float fireAngle = ship.rotation + 180.f;
            const Vec2 backVec = vec2RotationToDir(fireAngle);
            const Vec2 pos = ship.pos + backVec * 20.f;
            simulateParticleEmitter(visualWorld.particles, visualWorld.shipThrustEmitter, pos, fireAngle, dt);
        }
    }

    // particles
    std::vector<size_t> particlesToDeleteIndices{};

    for (size_t particleIndex = 0; particleIndex < visualWorld.particles.size(); ++particleIndex)
    {
        Particle& particle = visualWorld.particles[particleIndex];
        particle.lifetime += dt;

        if (particle.lifetime > particle.totalLifetime)
        {
            particlesToDeleteIndices.push_back(particleIndex);
            continue;
        }

        particle.pos += particle.velocity * dt;
    }

    for (int i = particlesToDeleteIndices.size() - 1; i >= 0; --i)
    {
        visualWorld.particles.erase(visualWorld.particles.begin() + particlesToDeleteIndices[i]);
    }

    for (const GameEventShipDeath& shipDeath : gameEvents.shipDeath)
    {
        const Ship ship = logicWorld.ships[shipDeath.shipIndex];
        for (int deadPieceIndex = 0; deadPieceIndex < 8; ++deadPieceIndex)
        {
            const Vec2 extraVelocityDir = vec2RotationToDir(90.f * 3 + deadPieceIndex * 360.f / 8.f);

            DeadShipPiece shipPiece;
            shipPiece.pos = ship.pos;
            shipPiece.velocity = ship.velocity;
            shipPiece.velocity += extraVelocityDir * randomFloatRange(0.f, 100.f);
            shipPiece.rotation = ship.rotation;
            shipPiece.angularSpeed = randomFloatRange(-30.f, 30.f);
            shipPiece.pieceIndex = deadPieceIndex;
            shipPiece.shipIndex = shipDeath.shipIndex;

            visualWorld.deadShipPieces.push_back(shipPiece);
        }
    }

    for (DeadShipPiece& shipPiece : visualWorld.deadShipPieces)
    {
        shipPiece.pos += shipPiece.velocity * dt;
        shipPiece.rotation += shipPiece.angularSpeed * dt;
    }
}
