#include "game_frame.h"
#include "game_logic.h"
#include "game_visual.h"

void gameFrameUpdate(entt::registry& registry, const float dt, const Vec2 worldSize)
{
    gravityWellSystem(registry, dt);
    rotateByInputSystem(registry);
    accelerateByInputSystem(registry, dt);
    accelerateImpulseSystem(registry, dt);
    applyRotationSpeedSystem(registry, dt);
    applyVelocitySystem(registry, dt);
    wrapPositionAroundWorldSystem(registry, worldSize);
    shootingSystem(registry, dt);
    projectileMoveSystem(registry, dt);
    circleVsCircleCollisionSystem(registry);
    teleportSystem(registry);

    spawnDeadShipPiecesOnCollisionSystem(registry);
    enableParticleEmitterByAccelerateInputSystem(registry);
    emitParticlesOnAccelerateImpulseSystem(registry);
    particleEmitterSystem(registry, dt);

    accelerateImpulseAppliedOneshotComponentClearSystem(registry);

    destroyByCollisionSystem(registry);
    destroyTimerSystem(registry, dt);
}
