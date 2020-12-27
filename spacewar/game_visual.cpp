#include "game_visual.h"

static void simulateParticlesEmitter(
    std::vector<Particle>& particles,
    ParticlesEmitter& emitter,
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

static void simulateParticles(std::vector<Particle>& particles, const float dt)
{
    std::vector<size_t> toDeleteIndices{};

    for (size_t particleIndex = 0; particleIndex < particles.size(); ++particleIndex)
    {
        Particle& particle = particles[particleIndex];
        particle.lifetime += dt;

        if (particle.lifetime > particle.totalLifetime)
        {
            toDeleteIndices.push_back(particleIndex);
            continue;
        }

        particle.pos += particle.velocity * dt;
    }

    for (int i = toDeleteIndices.size() - 1; i >= 0; --i)
    {
        particles.erase(particles.begin() + toDeleteIndices[i]);
    }
}

void gameVisualSimulate(GameVisualWorld& visualWorld, const GameWorld& logicWorld, float dt)
{
    for (const Ship& ship : logicWorld.ships)
    {
        if (ship.input.accelerate)
        {
            const float fireAngle = ship.rotation + 180.f;
            const Vec2 backVec = vec2RotationToDir(fireAngle);
            const Vec2 pos = ship.pos + backVec * 20.f;
            simulateParticlesEmitter(visualWorld.particles, visualWorld.shipThrustEmitter, pos, fireAngle, dt);
        }
    }

    simulateParticles(visualWorld.particles, dt);
}
