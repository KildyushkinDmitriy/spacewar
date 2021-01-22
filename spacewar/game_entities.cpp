#include "game_entities.h"
#include "game_visual.h"

entt::registry::entity_type createProjectileEntity(entt::registry& registry)
{
    const auto entity = registry.create();
    registry.emplace<DrawUsingShipTextureComponent>(entity, Vec2{10, 15}, sf::Color{255, 100, 100, 255});
    registry.emplace<WrapPositionAroundWorldComponent>(entity);
    registry.emplace<ProjectileComponent>(entity);
    registry.emplace<DestroyTimerComponent>(entity, 5.f);
    registry.emplace<DestroyByCollisionComponent>(entity);

    ParticleEmitterComponent& emitterComponent = registry.emplace<ParticleEmitterComponent>(entity);
    emitterComponent.isEnabled = true;

    ParticleEmitterSettings& emitterSettings = emitterComponent.settings;
    emitterSettings.particlesPerSec = 30.f;
    emitterSettings.angleRange = FloatRange{-5.f, 5.f};
    emitterSettings.speedRange = FloatRange{15.f, 17.f};
    emitterSettings.lifetimeRange = FloatRange{0.25f, 0.3f};
    emitterSettings.startRadiusRange = FloatRange{4.f, 5.f};
    emitterSettings.finishRadiusRange = FloatRange{1.f, 1.3f};
    emitterSettings.startColorRange = ColorRange{sf::Color{100, 0, 0, 150}, sf::Color{100, 0, 0, 150}};
    emitterSettings.finishColorRange = ColorRange{sf::Color{0, 0, 0, 0}, sf::Color{0, 0, 0, 150}};
    emitterSettings.emitOffset = 10.f;
    emitterSettings.emitAngleOffset = 180.f;

    return entity;
}

entt::registry::entity_type createShipEntity(entt::registry& registry, Vec2 position, float rotation, sf::Color color, int playerIndex)
{
    const auto entity = registry.create();
    registry.emplace<ShipComponent>(entity, playerIndex);
    registry.emplace<PositionComponent>(entity, position);
    registry.emplace<DrawUsingShipTextureComponent>(entity, Vec2{35.f, 35.f}, color);
    registry.emplace<VelocityComponent>(entity);
    registry.emplace<RotationComponent>(entity, rotation);
    registry.emplace<RotationSpeedComponent>(entity, 45.f);
    registry.emplace<AccelerateByInputComponent>(entity, false, 25.f);
    registry.emplace<RotateByInputComponent>(entity, 0.f, 180.f);
    registry.emplace<ShootingComponent>(entity, false, CooldownTimer{1.f}, 40.f, 200.f, createProjectileEntity);
    registry.emplace<WrapPositionAroundWorldComponent>(entity);
    registry.emplace<AccelerateImpulseByInputComponent>(entity, false, CooldownTimer{3.f}, 75.f);
    registry.emplace<CircleColliderComponent>(entity, 15.f);
    registry.emplace<DestroyByCollisionComponent>(entity);
    registry.emplace<SusceptibleToGravityWellComponent>(entity);
    registry.emplace<TeleportableComponent>(entity);

    {
        ParticleEmitterComponent& emitterComponent = registry.emplace<ParticleEmitterComponent>(entity);
        ParticleEmitterSettings& emitterSettings = emitterComponent.settings;
        emitterSettings.particlesPerSec = 30.f;
        emitterSettings.angleRange = FloatRange{-10.f, 10.f};
        emitterSettings.speedRange = FloatRange{180.f, 220.f};
        emitterSettings.lifetimeRange = FloatRange{0.4f, 0.6f};
        emitterSettings.startRadiusRange = FloatRange{7.f, 12.f};
        emitterSettings.finishRadiusRange = FloatRange{1.f, 2.f};
        emitterSettings.startColorRange = ColorRange{sf::Color{255, 0, 0, 150}, sf::Color{255, 150, 0, 150}};
        emitterSettings.finishColorRange = ColorRange{sf::Color{0, 0, 0, 0}, sf::Color{25, 25, 0, 150}};
        emitterSettings.emitOffset = 20.f;
        emitterSettings.emitAngleOffset = 180.f;

        registry.emplace<EnableParticleEmitterByAccelerateInputComponent>(entity);
    }

    {
        ParticleEmitterOnAccelerateImpulseComponent& emitterComponent = registry.emplace<ParticleEmitterOnAccelerateImpulseComponent>(entity);
        ParticleEmitterSettings& emitterSettings = emitterComponent.settings;
        emitterSettings.emitOffset = 17.f;
        emitterSettings.emitAngleOffset = 180.f;
        emitterSettings.particlesPerSpawn = 50;
        emitterSettings.angleRange = FloatRange{-30.f, 30.f};
        emitterSettings.speedRange = FloatRange{150.f, 300.f};
        emitterSettings.lifetimeRange = FloatRange{0.2f, 0.4f};
        emitterSettings.startRadiusRange = FloatRange{9.f, 15.f};
        emitterSettings.finishRadiusRange = FloatRange{2.f, 3.f};
        emitterSettings.startColorRange = ColorRange{sf::Color{255, 100, 0, 150}, sf::Color{255, 150, 0, 150}};
        emitterSettings.finishColorRange = ColorRange{sf::Color{0, 0, 0, 0}, sf::Color{25, 25, 0, 150}};
    }

    return entity;
}

entt::registry::entity_type createGravityWellEntity(entt::registry& registry, const Vec2 worldSize)
{
    const Vec2 pos = worldSize / 2.f;
    const float maxRadius = vec2Length(pos);

    const auto entity = registry.create();
    registry.emplace<PositionComponent>(entity, pos);

    GravityWellComponent wellComp;
    wellComp.maxRadius = maxRadius;
    wellComp.maxPower = 1500.f;
    wellComp.dragCoefficient = 0.3;
    wellComp.dragRadius = 50.f;
    registry.emplace<GravityWellComponent>(entity, wellComp);

    TeleportComponent teleport;
    teleport.destination = worldSize;
    teleport.radius = 10.f;
    teleport.speedAfterTeleport = 20.f;
    registry.emplace<TeleportComponent>(entity, teleport);

    return entity;
}

void createStarEntities(entt::registry& registry, const Vec2 worldSize)
{
    for (int i = 0; i < 50; ++i)
    {
        const auto entity = registry.create();

        const Vec2 pos{randomFloatRange(0, worldSize.x), randomFloatRange(0, worldSize.y)};
        registry.emplace<PositionComponent>(entity, pos);

        StarComponent& starComponent = registry.emplace<StarComponent>(entity);
        starComponent.radius = randomFloatRange(0.5f, 2.f);
        starComponent.brightnessRange = FloatRange{randomFloatRange(0.2f, 0.6f), randomFloatRange(0.6f, 1.f)};
        starComponent.brightnessPeriodsPerSec = randomFloatRange(0.2f, 0.5f);
    }
}

void recreateGameWorld(entt::registry& registry, std::vector<Player>& players, const Vec2 worldSize)
{
    registry.clear();

    const auto shipEntity1 = createShipEntity(registry, worldSize / 2.f - worldSize / 4.f, 180.f + 45.f, sf::Color::Cyan, 0);
    const auto shipEntity2 = createShipEntity(registry, worldSize / 2.f + worldSize / 4.f, 45.f, sf::Color::White, 1);

    createGravityWellEntity(registry, worldSize);
    createStarEntities(registry, worldSize);

    players[0].shipEntity = shipEntity1;
    players[1].shipEntity = shipEntity2;
}
