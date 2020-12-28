#include "game_visual.h"

static void emitParticle(
    std::vector<Particle>& particles,
    const ParticleEmitterSettings& settings,
    const Vec2 pos,
    const float baseAngle
)
{
    for (int i = 0; i < settings.particlesPerSpawn; ++i)
    {
        Particle particle;
        particle.pos = pos;

        const float angle = baseAngle + settings.angleRange.getRandom();
        const Vec2 dir = vec2AngleToDir(angle);
        const float speed = settings.speedRange.getRandom();

        particle.velocity = dir * speed;
        particle.totalLifetime = settings.lifetimeRange.getRandom();

        particle.startRadius = settings.startRadiusRange.getRandom();
        particle.finishRadius = settings.finishRadiusRange.getRandom();

        particle.startColor = settings.startColorRange.getRandom();
        particle.finishColor = settings.finishColorRange.getRandom();

        particles.push_back(particle);
    }
}

static void simulateParticleEmitter(
    std::vector<Particle>& particles,
    ParticleEmitter& emitter,
    const ParticleEmitterSettings& settings,
    const Vec2 pos,
    const float baseAngle,
    const float dt
)
{
    const float timeBetweenParticles = 1.f / settings.particlesPerSec;
    emitter.timer += dt;

    while (emitter.timer > timeBetweenParticles)
    {
        emitter.timer -= timeBetweenParticles;
        emitParticle(particles, settings, pos, baseAngle);
    }
}

std::pair<Vec2, float> getPosBehind(const Vec2& pos, const float rotation, const float offset)
{
    const float invAngle = rotation + 180.f;
    const Vec2 backVec = vec2AngleToDir(invAngle);
    const Vec2 resultPos = pos + backVec * offset;
    return {resultPos, invAngle};
}

void gameVisualSimulate(GameVisualWorld& visualWorld, const GameWorld& logicWorld, const GameEvents& gameEvents,
                        const float dt)
{
    // ship thrust emitters
    for (int i = 0; i < visualWorld.shipThrustParticleEmitters.size(); ++i)
    {
        const Ship& ship = logicWorld.ships[i];

        if (ship.input.thrust)
        {
            ParticleEmitter& emitter = visualWorld.shipThrustParticleEmitters[i];
            auto [pos, angle] = getPosBehind(ship.pos, ship.rotation, 20.f);
            simulateParticleEmitter(visualWorld.particles, emitter, visualWorld.shipThrustEmitterSettings, pos, angle, dt);
        }
    }

    // Projectile emitters
    for (const GameEventProjectileCreated event : gameEvents.projectileCreate)
    {
        visualWorld.projectileParticleEmitters.insert(visualWorld.projectileParticleEmitters.begin() + event.projectileIndex,1, {});
    }
    
    for (const GameEventProjectileDestroyed event : gameEvents.projectileDestroyed)
    {
        visualWorld.projectileParticleEmitters.erase(visualWorld.projectileParticleEmitters.begin() + event.projectileIndex);
    }
    
    for (int i = 0; i < visualWorld.projectileParticleEmitters.size(); ++i)
    {
        ParticleEmitter& emitter = visualWorld.projectileParticleEmitters[i];
        const Projectile& projectile = logicWorld.projectiles[i];

        auto [pos, angle] = getPosBehind(projectile.pos, projectile.rotation, 10.f);
        simulateParticleEmitter(visualWorld.particles, emitter, visualWorld.projectileTrailEmitterSettings, pos, angle, dt);
    }
        
    // ship thrust burst emit
    for (const GameEventShipThrustBurst& burst : gameEvents.shipThrustBurst)
    {
        const Ship ship = logicWorld.ships[burst.shipIndex];
        auto [pos, angle] = getPosBehind(ship.pos, ship.rotation, 17.f);
        emitParticle(visualWorld.particles, visualWorld.shipThrustBurstEmitSettings, pos, angle);
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

    // dead ship pieces
    for (const GameEventShipDeath& shipDeath : gameEvents.shipDeath)
    {
        const Ship ship = logicWorld.ships[shipDeath.shipIndex];
        for (int deadPieceIndex = 0; deadPieceIndex < 8; ++deadPieceIndex)
        {
            const Vec2 extraVelocityDir = vec2AngleToDir(90.f * 3 + deadPieceIndex * 360.f / 8.f);

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
