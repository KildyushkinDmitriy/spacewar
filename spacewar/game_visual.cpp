#include "game_visual.h"

static void emitParticles(entt::registry& registry, const ParticleEmitterSettings& settings, const PositionComponent position, const RotationComponent rotationComponent)
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

void particleEmitterSystem(entt::registry& registry, const float dt)
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
        emitter.isEnabled = accelerate.input;
    }
}

void emitParticlesOnAccelerateImpulseSystem(entt::registry& registry)
{
    const auto view = registry.view<
        ParticleEmitterOnAccelerateImpulseComponent,
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
        const CollisionHappenedOneshotComponent>();

    for (auto [_, position, rotation, velocity, drawIn, ship] : view.each())
    {
        // Copy the data from view references. Because emplacing new components of the same types will invalidate the references  
        const Vec2 pos = position.vec;
        const float angle = rotation.angle;
        DrawUsingShipTextureComponent draw = drawIn;

        for (int deadPieceIndex = 0; deadPieceIndex < DEAD_SHIP_PIECES_COUNT; ++deadPieceIndex)
        {
            const Vec2 extraVelocityDir = vec2AngleToDir(90.f * 3 + deadPieceIndex * 360.f / 8.f);
            const Vec2 velocityVec = velocity.vec + extraVelocityDir * randomFloatRange(0.f, 100.f);
            const float angularSpeed = randomFloatRange(-30.f, 30.f);

            const auto pieceEntity = registry.create();

            registry.emplace<PositionComponent>(pieceEntity, pos);
            registry.emplace<VelocityComponent>(pieceEntity, velocityVec);

            registry.emplace<RotationComponent>(pieceEntity, angle);
            registry.emplace<RotationSpeedComponent>(pieceEntity, angularSpeed);

            registry.emplace<DeadShipPieceComponent>(pieceEntity, deadPieceIndex);
            registry.emplace<DrawUsingShipTextureComponent>(pieceEntity, draw);

            registry.emplace<WrapPositionAroundWorldComponent>(pieceEntity);
        }
    }
}
