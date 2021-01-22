#include "game_visual.h"

static void emitParticles(
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
        emitParticles(particles, settings, pos, baseAngle);
    }
}

std::pair<Vec2, float> getBackwardPosAndAngle(const Vec2& pos, const float rotation, const float offset)
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
    for (size_t i = 0; i < visualWorld.shipThrustParticleEmitters.size(); ++i)
    {
        const Ship& ship = logicWorld.ships[i];

        if (ship.input.thrust)
        {
            ParticleEmitter& emitter = visualWorld.shipThrustParticleEmitters[i];
            auto [pos, angle] = getBackwardPosAndAngle(ship.pos, ship.rotation, 20.f);
            simulateParticleEmitter(visualWorld.particles, emitter, visualWorld.shipThrustEmitterSettings, pos, angle,
                                    dt);
        }
    }

    // Projectile emitters
    for (const GameEventProjectileCreated event : gameEvents.projectileCreate)
    {
        visualWorld.projectileParticleEmitters.insert(
            visualWorld.projectileParticleEmitters.begin() + event.projectileIndex, 1, {});
    }

    for (const GameEventProjectileDestroyed event : gameEvents.projectileDestroyed)
    {
        visualWorld.projectileParticleEmitters.erase(
            visualWorld.projectileParticleEmitters.begin() + event.projectileIndex);
    }

    for (size_t i = 0; i < visualWorld.projectileParticleEmitters.size(); ++i)
    {
        ParticleEmitter& emitter = visualWorld.projectileParticleEmitters[i];
        const Projectile& projectile = logicWorld.projectiles[i];

        auto [pos, angle] = getBackwardPosAndAngle(projectile.pos, projectile.rotation, 10.f);
        simulateParticleEmitter(visualWorld.particles, emitter, visualWorld.projectileTrailEmitterSettings, pos, angle,
                                dt);
    }

    // ship thrust burst emit
    for (const GameEventShipThrustBurst& burst : gameEvents.shipThrustBurst)
    {
        const Ship ship = logicWorld.ships[burst.shipIndex];
        auto [pos, angle] = getBackwardPosAndAngle(ship.pos, ship.rotation, 17.f);
        emitParticles(visualWorld.particles, visualWorld.shipThrustBurstEmitSettings, pos, angle);
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

static void emitParticles(
    entt::registry& registry,
    const ParticleEmitterSettings& settings,
    const PositionComponent position,
    const RotationComponent rotationComponent
)
{
    const float baseAngle = rotationComponent.angle + settings.emitAngleOffset;
    const Vec2 emitDir = vec2AngleToDir(baseAngle);
    const Vec2 pos = position.vec + emitDir * settings.emitOffset;

    for (int i = 0; i < settings.particlesPerSpawn; ++i)
    {
        const float angle = baseAngle + settings.angleRange.getRandom();
        const Vec2 dir = vec2AngleToDir(angle);
        const float speed = settings.speedRange.getRandom();
        const float lifetime = settings.lifetimeRange.getRandom();

        const auto entity = registry.create();

        registry.emplace<PositionComponent>(entity, pos);
        registry.emplace<VelocityComponent>(entity, dir * speed);

        ParticleComponent particleComponent;
        particleComponent.totalLifetime = lifetime;
        particleComponent.startRadius = settings.startRadiusRange.getRandom();
        particleComponent.finishRadius = settings.finishRadiusRange.getRandom();
        particleComponent.startColor = settings.startColorRange.getRandom();
        particleComponent.finishColor = settings.finishColorRange.getRandom();

        registry.emplace<ParticleComponent>(entity, particleComponent);

        registry.emplace<DestroyTimerComponent>(entity, lifetime);
    }
}

void particleEmitterSystem(entt::registry& registry, float dt)
{
    const auto view = registry.view<ParticleEmitterComponent, const PositionComponent, const RotationComponent>();

    for (auto [_, emitter, position, rotation] : view.each())
    {
        if (!emitter.isEnabled)
        {
            continue;
        }

        const float timeBetweenParticles = 1.f / emitter.settings.particlesPerSec;
        emitter.timer += dt;

        while (emitter.timer > timeBetweenParticles)
        {
            emitter.timer -= timeBetweenParticles;
            emitParticles(registry, emitter.settings, position, rotation);
        }
    }
}

void enableParticleEmitterByAccelerateInputSystem(entt::registry& registry)
{
    const auto view = registry.view<ParticleEmitterComponent, const AccelerateByInputComponent>();

    for (auto [_, emitter, accelerate] : view.each())
    {
        emitter.isEnabled = accelerate.accelerateInput;
    }
}

void particleEmitterOnAccelerateImpulseAppliedSystem(entt::registry& registry)
{
    const auto view = registry.view<
        ParticleEmitterOnAccelerateImpulseAppliedComponent,
        const PositionComponent,
        const RotationComponent,
        const AccelerateImpulseAppliedOneshotComponent>();

    for (auto [_, emitter, position, rotation] : view.each())
    {
        emitParticles(registry, emitter.settings, position, rotation);
    }
}


void spawnDeadShipPiecesOnCollisionSystem(entt::registry& registry)
{
    const auto view = registry.view<
        const PositionComponent,
        const RotationComponent,
        const VelocityComponent,
        const DrawUsingShipTextureComponent,
        const ShipComponent,
        const CollisionHappenedComponent>();


    for (auto [_, position, rotation, velocity, drawIn] : view.each())
    {
        const Vec2 pos = position.vec;
        const float angle = rotation.angle;
        DrawUsingShipTextureComponent draw = drawIn;
        
        // TODO: 8 is magic number
        for (int deadPieceIndex = 0; deadPieceIndex < 8; ++deadPieceIndex)
        {
            const Vec2 extraVelocityDir = vec2AngleToDir(90.f * 3 + deadPieceIndex * 360.f / 8.f);
            const Vec2 velocityVec = velocity.vec + extraVelocityDir * randomFloatRange(0.f, 100.f);
            const float angularSpeed = randomFloatRange(-30.f, 30.f);
            
            const auto pieceEntity = registry.create();

            registry.emplace<PositionComponent>(pieceEntity, pos);
            registry.emplace<VelocityComponent>(pieceEntity, velocityVec);

            registry.emplace<RotationComponent>(pieceEntity, angle);
            registry.emplace<AngularSpeedComponent>(pieceEntity, angularSpeed);

            registry.emplace<DeadShipPieceComponent>(pieceEntity, deadPieceIndex);
            registry.emplace<DrawUsingShipTextureComponent>(pieceEntity, draw);
        }
    }
}
