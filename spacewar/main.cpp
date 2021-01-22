#include "game_logic.h"
#include "game_visual.h"
#include "player.h"
#include "render.h"
#include "entt.hpp"

#include <SFML/Graphics.hpp>
#include <variant>
#include <cassert>

void runTests();

GameWorld createWorld(const Vec2 size)
{
    GameWorld world;
    world.size = size;

    Ship ship1;
    ship1.pos = world.size / 2.f - size / 4.f;
    ship1.rotation = 180.f + 45.f;
    world.ships.push_back(ship1);

    Ship ship2;
    ship2.pos = world.size / 2.f + size / 4.f;
    ship2.rotation = 45.f;
    world.ships.push_back(ship2);

    GravityWell well;
    well.pos = size / 2.f;
    well.maxRadius = vec2Length(well.pos);
    well.maxPower = 1500.f;
    well.dragRadius = 50.f;
    world.gravityWells.push_back(well);

    world.settings.shipThrustAcceleration = 25.f;
    world.settings.shipThrustBurstImpulse = 75.f;
    world.settings.shipThrustBurstImpulseCooldown = 3.f;
    world.settings.shipRotationSpeed = 180.f;
    world.settings.shootCooldown = 1.f;
    world.settings.projectileSpeed = 200.f;
    world.settings.projectileLifetime = 5.f;
    world.settings.shipCollisionRadius = 15.f;
    world.settings.muzzleExtraOffset = 25.f;
    world.settings.gravityWellDragCoefficient = 0.3f;

    return world;
}

GameVisualWorld createVisualWorld(const GameWorld& logicWorld)
{
    GameVisualWorld visualWorld;

    ParticleEmitterSettings& shipThrustEmitter = visualWorld.shipThrustEmitterSettings;
    shipThrustEmitter.particlesPerSec = 30.f;
    shipThrustEmitter.angleRange = FloatRange{-10.f, 10.f};
    shipThrustEmitter.speedRange = FloatRange{180.f, 220.f};
    shipThrustEmitter.lifetimeRange = FloatRange{0.4f, 0.6f};
    shipThrustEmitter.startRadiusRange = FloatRange{7.f, 12.f};
    shipThrustEmitter.finishRadiusRange = FloatRange{1.f, 2.f};
    shipThrustEmitter.startColorRange = ColorRange{sf::Color{255, 0, 0, 150}, sf::Color{255, 150, 0, 150}};
    shipThrustEmitter.finishColorRange = ColorRange{sf::Color{0, 0, 0, 0}, sf::Color{25, 25, 0, 150}};

    ParticleEmitterSettings& shipThrustBurstEmitter = visualWorld.shipThrustBurstEmitSettings;
    shipThrustBurstEmitter.particlesPerSpawn = 50;
    shipThrustBurstEmitter.angleRange = FloatRange{-30.f, 30.f};
    shipThrustBurstEmitter.speedRange = FloatRange{150.f, 300.f};
    shipThrustBurstEmitter.lifetimeRange = FloatRange{0.2f, 0.4f};
    shipThrustBurstEmitter.startRadiusRange = FloatRange{9.f, 15.f};
    shipThrustBurstEmitter.finishRadiusRange = FloatRange{2.f, 3.f};
    shipThrustBurstEmitter.startColorRange = ColorRange{sf::Color{255, 100, 0, 150}, sf::Color{255, 150, 0, 150}};
    shipThrustBurstEmitter.finishColorRange = ColorRange{sf::Color{0, 0, 0, 0}, sf::Color{25, 25, 0, 150}};

    ParticleEmitterSettings& projectileTrailEmitter = visualWorld.projectileTrailEmitterSettings;
    projectileTrailEmitter.particlesPerSec = 30.f;
    projectileTrailEmitter.angleRange = FloatRange{-5.f, 5.f};
    projectileTrailEmitter.speedRange = FloatRange{15.f, 17.f};
    projectileTrailEmitter.lifetimeRange = FloatRange{0.25f, 0.3f};
    projectileTrailEmitter.startRadiusRange = FloatRange{4.f, 5.f};
    projectileTrailEmitter.finishRadiusRange = FloatRange{1.f, 1.3f};
    projectileTrailEmitter.startColorRange = ColorRange{sf::Color{100, 0, 0, 150}, sf::Color{100, 0, 0, 150}};
    projectileTrailEmitter.finishColorRange = ColorRange{sf::Color{0, 0, 0, 0}, sf::Color{0, 0, 0, 150}};

    visualWorld.shipThrustParticleEmitters.resize(logicWorld.ships.size());

    for (int i = 0; i < 50; ++i)
    {
        Star star;
        star.pos = Vec2{randomFloatRange(0, logicWorld.size.x), randomFloatRange(0, logicWorld.size.y)};
        star.radius = randomFloatRange(0.5f, 2.f);
        star.brightnessRange = FloatRange{randomFloatRange(0.2f, 0.6f), randomFloatRange(0.6f, 1.f)};
        star.periodsPerSec = randomFloatRange(0.2f, 0.5f);
        visualWorld.stars.push_back(star);
    }

    return visualWorld;
}

void startingStateSfEventHandler(AppStateStarting& startingState, std::vector<Player>& players, const sf::Event& event)
{
    if (event.type != sf::Event::KeyReleased)
    {
        return;
    }

    for (size_t i = 0; i < players.size(); ++i)
    {
        Player& player = players[i];
        forEachKeyInKeymap(player.keymap, [&event, &startingState, i](const sf::Keyboard::Key key)
        {
            if (event.key.code == key)
            {
                startingState.playersReady[i] = true;
            }
        });

        if (event.key.code == player.makeAiKey)
        {
            player.isAi = true;
            startingState.playersReady[i] = true;
        }
    }
}

void update(entt::registry& registry, const float dt, const Vec2 worldSize)
{
    gravityWellSystem(registry, dt);
    rotateByInputSystem(registry);
    accelerateByInputSystem(registry, dt);
    accelerateImpulseSystem(registry, dt);
    angularSpeedIntegrateSystem(registry, dt);
    integrateVelocitySystem(registry, dt);
    wrapPositionAroundWorldSystem(registry, worldSize);
    shootingSystem(registry, dt);
    projectileMoveSystem(registry, dt);
    circleVsCircleCollisionSystem(registry);
    teleportSystem(registry);

    spawnDeadShipPiecesOnCollisionSystem(registry);
    enableParticleEmitterByAccelerateInputSystem(registry);
    particleEmitterOnAccelerateImpulseAppliedSystem(registry);
    particleEmitterSystem(registry, dt);
        
    accelerateImpulseAppliedOneshotComponentClearSystem(registry);
    
    destroyByCollisionSystem(registry);
    destroyTimerSystem(registry, dt);
}

entt::registry::entity_type createProjectileEntity(entt::registry& registry)
{
    const auto entity = registry.create();
    registry.emplace<DrawUsingShipTextureComponent>(entity, Vec2{10, 15}, sf::Color{255, 100, 100, 255});
    registry.emplace<WrapPositionAroundWorldComponent>(entity);
    registry.emplace<ProjectileComponent>(entity);
    registry.emplace<DestroyTimerComponent>(entity, 5.f);

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

entt::registry::entity_type createShipEntity(entt::registry& registry, Vec2 position)
{
    const auto entity = registry.create();
    registry.emplace<ShipComponent>(entity);
    registry.emplace<PositionComponent>(entity, position);
    registry.emplace<DrawUsingShipTextureComponent>(entity, Vec2{35.f, 35.f}, sf::Color::Green);
    registry.emplace<VelocityComponent>(entity);
    registry.emplace<RotationComponent>(entity, 45.f);
    registry.emplace<AngularSpeedComponent>(entity, 45.f);
    registry.emplace<AccelerateByInputComponent>(entity, false, 25.f);
    registry.emplace<RotateByInputComponent>(entity, 0.f, 180.f);
    registry.emplace<ShootingComponent>(entity, false, CooldownTimer{1.f}, 40.f, 200.f, createProjectileEntity);
    registry.emplace<WrapPositionAroundWorldComponent>(entity);
    registry.emplace<AccelerateImpulseByInputComponent>(entity, false, CooldownTimer{3.f}, 75.f);
    registry.emplace<CircleColliderComponent>(entity, 15.f);
    registry.emplace<DestroyByCollisionComponent>(entity);
    registry.emplace<AffectedByGravityWellComponent>(entity);
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
        ParticleEmitterOnAccelerateImpulseAppliedComponent& emitterComponent = registry.emplace<ParticleEmitterOnAccelerateImpulseAppliedComponent>(entity);
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

int main()
{
    runTests();

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window{sf::VideoMode{1000, 1000}, "Spacewar!", sf::Style::Default, settings};

    sf::Texture shipTexture;
    if (!shipTexture.loadFromFile("images/ship.png"))
    {
        return EXIT_FAILURE;
    }

    sf::Font font;
    if (!font.loadFromFile("fonts/arial.ttf"))
    {
        return EXIT_FAILURE;
    }

    GameWorld world;
    GameVisualWorld visualWorld;

    const auto initWorlds = [&world, &visualWorld, &window]()
    {
        world = createWorld(Vec2{window.getSize()});
        visualWorld = createVisualWorld(world);
    };

    initWorlds();

    entt::registry registry;

    const auto shipEntity = createShipEntity(registry, world.size / 2.f);
    createShipEntity(registry, world.size / 3.f);

    createGravityWellEntity(registry, world.size);

    std::vector<Player> players{
        {
            PlayerKeymap{
                sf::Keyboard::A, sf::Keyboard::D, sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::LShift
            },
            "WASD",
            sf::Keyboard::Q
        },
        {
            PlayerKeymap{
                sf::Keyboard::J, sf::Keyboard::L, sf::Keyboard::I, sf::Keyboard::K, sf::Keyboard::RShift
            },
            "IJKL",
            sf::Keyboard::O
        }
    };

    AppState appState = AppStateGame{};
    // std::get<AppStateStarting>(appState).playersReady.resize(players.size(), false);

    bool isDebugRender = false;

    sf::Clock timer;
    while (window.isOpen())
    {
        const float dt = std::clamp(timer.restart().asSeconds(), 0.f, 0.1f);

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tilde)
            {
                isDebugRender = !isDebugRender;
            }

            if (auto* startingState = std::get_if<AppStateStarting>(&appState))
            {
                startingStateSfEventHandler(*startingState, players, event);
            }
        }

        assert(world.ships.size() == players.size());

        // update
        if (std::holds_alternative<AppStateGame>(appState))
        {
            // player input
            for (size_t i = 0; i < players.size(); ++i)
            {
                const Player& player = players[i];
                world.ships[i].input = player.isAi
                                           ? aiGenerateInput(world, i, (i + 1) % players.size())
                                           : readPlayerInput(player.keymap);
            }

            {
                auto playerInputForEcs = readPlayerInput(players[0].keymap);

                if (registry.valid(shipEntity))
                {
                    registry.get<AccelerateByInputComponent>(shipEntity).accelerateInput = playerInputForEcs.thrust;
                    registry.get<AccelerateByInputComponent>(shipEntity).accelerateInput = playerInputForEcs.thrust;
                    registry.get<RotateByInputComponent>(shipEntity).input = playerInputForEcs.rotate;
                    registry.get<ShootingComponent>(shipEntity).input = playerInputForEcs.shoot;
                    registry.get<AccelerateImpulseByInputComponent>(shipEntity).input = playerInputForEcs.thrustBurst;
                }
            }

            update(registry, dt, world.size);

            // const GameEvents gameEvents = gameSimulate(world, dt);
            // gameVisualSimulate(visualWorld, world, gameEvents, dt);

            const GameEvents gameEvents{};
            
            if (gameEvents.result.has_value())
            {
                if (!gameEvents.result->isTie())
                {
                    players[gameEvents.result->victoriousPlayerIndex].score++;
                }

                appState = AppStateGameOver{gameEvents.result.value(), 15.f};
            }
        }
        else if (auto* gameOverState = std::get_if<AppStateGameOver>(&appState))
        {
            gameOverState->timeInState += dt;

            for (Ship& ship : world.ships)
            {
                ship.input = {};
            }

            constexpr float timeInSlowMotion = 2.f;
            const float t = std::clamp(gameOverState->timeInState / timeInSlowMotion, 0.f, 1.f);
            const float slowMotionMultiplier = floatLerp(0.2f, 1.f, t);
            const float slowMotionDt = slowMotionMultiplier * dt;

            const GameEvents gameEvents = gameSimulate(world, slowMotionDt);
            gameVisualSimulate(visualWorld, world, gameEvents, slowMotionDt);

            const bool restartButtonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
            if (restartButtonPressed || gameOverState->timeInState > gameOverState->timeWhenRestart)
            {
                initWorlds();
                appState = AppStateGame{};
            }
        }
        else if (auto* startingState = std::get_if<AppStateStarting>(&appState))
        {
            startingState->timeInState += dt;
            std::vector<bool>& readyVec = startingState->playersReady;
            const bool everyoneReady = std::all_of(readyVec.begin(), readyVec.end(), [](const bool ready)
            {
                return ready;
            });
            if (everyoneReady)
            {
                initWorlds();
                appState = AppStateGame{};
            }
        }

        // render
        window.clear(sf::Color{5, 10, 30, 255});
        if (std::holds_alternative<AppStateGame>(appState) || std::holds_alternative<AppStateGameOver>(appState))
        {
            if (isDebugRender)
            {
                renderGameDebug(world, window, font);
            }
            else
            {
                renderGame(world, visualWorld, window, shipTexture, registry);
            }
        }

        if (const auto* gameOverState = std::get_if<AppStateGameOver>(&appState))
        {
            renderGameOverUi(*gameOverState, players, window, font);
        }
        else if (const auto* startingState = std::get_if<AppStateStarting>(&appState))
        {
            renderStartingUi(*startingState, players, window, font);
        }

        window.display();
    }

    return EXIT_SUCCESS;
}
