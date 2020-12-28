#include "game_visual.h"

static void emitParticle(
    std::vector<Particle>& particles,
    const ParticleEmitSettings& emitter,
    const Vec2 pos,
    const float baseAngle
)
{
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

        emitParticle(particles, emitter.settings, pos, baseAngle);
    }
}

std::pair<Vec2, float> getPosBehind(const Vec2& pos, const float rotation, const float offset)
{
    const float invAngle = rotation + 180.f;
    const Vec2 backVec = vec2RotationToDir(invAngle);
    const Vec2 resultPos = pos + backVec * offset;
    return {resultPos, invAngle};
}

void gameVisualSimulate(GameVisualWorld& visualWorld, const GameWorld& logicWorld, const GameEvents& gameEvents,
                        const float dt)
{
    // ship thrust emitters
    for (const Ship& ship : logicWorld.ships)
    {
        if (ship.input.thrust)
        {
            auto [pos, angle] = getPosBehind(ship.pos, ship.rotation, 20.f);
            simulateParticleEmitter(visualWorld.particles, visualWorld.shipThrustEmitter, pos, angle, dt);
        }
    }

    // projectile trail emitters
    for (const Projectile& projectile : logicWorld.projectiles)
    {
        auto [pos, angle] = getPosBehind(projectile.pos, projectile.rotation, 10.f);
        simulateParticleEmitter(visualWorld.particles, visualWorld.projectileTrailEmitter, pos, angle, dt);
    }

    // ship thrust burst emit
    for (const GameEventShipThrustBurst& burst : gameEvents.shipThrustBurst)
    {
        const Ship ship = logicWorld.ships[burst.shipIndex];
        auto [pos, angle] = getPosBehind(ship.pos, ship.rotation, 17.f);
        ParticleBurstEmitter& emitter = visualWorld.shipThrustBurstEmitSettings;
        for (int i = 0; i < emitter.particlesCount; ++i)
        {
            emitParticle(visualWorld.particles, emitter.settings, pos, angle);
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
