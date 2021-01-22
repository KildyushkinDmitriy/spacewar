#include <assert.h>

#include "game_entities.h"
#include "game_frame.h"
#include "game_logic.h"

static void testFloatWrap()
{
    assert(floatEq(floatWrap(0.f, 1.f), 0.f));
    assert(floatEq(floatWrap(0.5f, 1.f), 0.5f));
    assert(floatEq(floatWrap(-0.5f, 1.f), 0.5f));
    assert(floatEq(floatWrap(-22.1f, 1.f), 0.9f));
    assert(floatEq(floatWrap(22.2f, 1.f), 0.2f));
}

static void testIsPointInsideCircle()
{
    assert(isPointInsideCircle(Vec2{0.f, 0.f}, Vec2{0.f, 0.f}, 0.1f));
    assert(!isPointInsideCircle(Vec2{1.f, 1.f}, Vec2{0.f, 0.f}, 0.1f));
    assert(isPointInsideCircle(Vec2{1.f, 1.f}, Vec2{0.f, 0.f}, 2.f));
    assert(!isPointInsideCircle(Vec2{1.f, 1.f}, Vec2{3.f, 0.f}, 2.f));
    assert(isPointInsideCircle(Vec2{-5.f, 0.f}, Vec2{0.f, 0.f}, 5.1f));
    assert(!isPointInsideCircle(Vec2{-5.2f, 0.f}, Vec2{0.f, 0.f}, 5.1f));

    const Vec2 point = vec2AngleToDir(45.f) * 3.f;

    assert(isPointInsideCircle(point, Vec2{0.f, 0.f}, 3.1f));
    assert(!isPointInsideCircle(point, Vec2{0.f, 0.f}, 2.9f));
}

static void testIsPointOnSegment()
{
    assert(isPointOnSegment(Vec2{0.5f, 0.f}, Vec2{0.f, 0.f}, Vec2{1.f, 0.f}));
    assert(!isPointOnSegment(Vec2{1.1f, 0.f}, Vec2{0.f, 0.f}, Vec2{1.f, 0.f}));
    assert(isPointOnSegment(Vec2{-5.f, -5.f}, Vec2{-2.f, -2.f}, Vec2{-6.f, -6.f}));
    assert(!isPointOnSegment(Vec2{5.f, -5.f}, Vec2{-2.f, -2.f}, Vec2{-6.f, -6.f}));
}

static void testIsSegmentIntersectCircle()
{
    assert(isSegmentIntersectCircle(Vec2{0.f, 0.f}, Vec2{1.f, 0.f}, Vec2{0.f, 0.f}, 1.f));
    assert(isSegmentIntersectCircle(Vec2{-1.f, 0.5f}, Vec2{1.f, 0.5f}, Vec2{0.f, 0.f}, 1.f));
    assert(!isSegmentIntersectCircle(Vec2{-1.f, 1.f}, Vec2{1.f, 4.f}, Vec2{0.f, 0.f}, 1.f));
    assert(isSegmentIntersectCircle(Vec2{-1.f, 0.5f}, Vec2{1.f, 0.5f}, Vec2{0.f, 0.f}, 1.f));
    assert(isSegmentIntersectCircle(Vec2{8.f, 1.f}, Vec2{12.f, 8.f}, Vec2{10.f, 5.f}, 3.f));
    assert(!isSegmentIntersectCircle(Vec2{8.f, 1.f}, Vec2{-12.f, 8.f}, Vec2{10.f, 5.f}, 3.f));
}

static void testIsCircleIntersectCircle()
{
    assert(isCircleIntersectCircle(Vec2{0.f, 0.f}, 1.f, Vec2{0.0f, 0.f}, 1.f));
    assert(isCircleIntersectCircle(Vec2{0.f, 0.f}, 1.f, Vec2{0.5f, 0.f}, 1.f));
    assert(!isCircleIntersectCircle(Vec2{0.f, 0.f}, 1.f, Vec2{3.f, 0.f}, 1.f));
    assert(isCircleIntersectCircle(Vec2{-5.f, -5.f}, 5.f, Vec2{0.0f, 0.f}, 3.f));
}

static void testColorLerp()
{
    const ColorRange range = ColorRange{sf::Color{255, 0, 0, 150}, sf::Color{255, 255, 0, 150}};
    const sf::Color interpolated = colorLerp(range.min, range.max, 0.5f);
    const sf::Color expected = sf::Color{255, 255 / 2, 0, 150};
    assert(expected == interpolated);
}

static void testProjectileKillsShip()
{
    const Vec2 worldSize{100.f, 100.f};
    const int playersCount = 1;

    entt::registry registry;

    const auto shipEnt = createShipEntity(registry, Vec2{10.f, 10.f}, 0.f, sf::Color::White, -1);

    const auto pjlEnt = createProjectileEntity(registry);
    registry.emplace<PositionComponent>(pjlEnt, Vec2{20.f, 14.f});
    registry.emplace<VelocityComponent>(pjlEnt, Vec2{-10.f, 0.f});

    gameFrameUpdate(registry, 10.f, worldSize);

    const std::optional<GameEventGameResult> optGameResult = tryGetGameResult(registry, playersCount);

    assert(optGameResult.has_value());
    assert(optGameResult->isTie());

    assert(!registry.valid(shipEnt));
    assert(!registry.valid(pjlEnt));
}

static void testShipsKillEachOtherWithProjectiles()
{
    const Vec2 worldSize{100.f, 100.f};
    const int playersCount = 2;

    entt::registry registry;

    const auto ship1 = createShipEntity(registry, Vec2{10.f, 0.f}, 0.f, sf::Color::White, 0);
    registry.get<ShootingComponent>(ship1).input = true;

    const auto ship2 = createShipEntity(registry, Vec2{90.f, 0.f}, 180.f, sf::Color::White, 0);
    registry.get<ShootingComponent>(ship2).input = true;

    {
        gameFrameUpdate(registry, 0.f, worldSize);
        const std::optional<GameEventGameResult> optGameResult = tryGetGameResult(registry, playersCount);
        assert(!optGameResult.has_value());
    }

    {
        gameFrameUpdate(registry, 3.f, worldSize);
        const std::optional<GameEventGameResult> optGameResult = tryGetGameResult(registry, playersCount);
        assert(optGameResult.has_value());
        assert(optGameResult->isTie());
    }
}

static void testShipShipCollisionKillsBoth()
{
    const Vec2 worldSize{100.f, 100.f};
    const int playersCount = 2;

    entt::registry registry;

    const auto ship1 = createShipEntity(registry, Vec2{10.f, 0.f}, 0.f, sf::Color::White, 0);
    registry.get<AccelerateByInputComponent>(ship1).input = true;

    const auto ship2 = createShipEntity(registry, Vec2{90.f, 0.f}, 180.f, sf::Color::White, 0);
    registry.get<AccelerateByInputComponent>(ship2).input = true;

    {
        gameFrameUpdate(registry, 0.f, worldSize);
        const std::optional<GameEventGameResult> optGameResult = tryGetGameResult(registry, playersCount);
        assert(!optGameResult.has_value());
    }

    std::optional<GameEventGameResult> optGameResult;

    for (int i = 0; i < 20; ++i)
    {
        gameFrameUpdate(registry, 0.1f, worldSize);
        optGameResult = tryGetGameResult(registry, playersCount);

        if (optGameResult)
        {
            break;
        }
    }

    assert(optGameResult.has_value());
    assert(optGameResult->isTie());
}

static void testPlayerWinsGameWithKill()
{
    const Vec2 worldSize{100.f, 100.f};
    const int playersCount = 2;

    entt::registry registry;

    const auto ship1 = createShipEntity(registry, Vec2{10.f, 0.f}, 0.f, sf::Color::White, 0);
    registry.get<ShootingComponent>(ship1).input = true;
    registry.remove<CircleColliderComponent>(ship1);

    const auto ship2 = createShipEntity(registry, Vec2{90.f, 0.f}, 180.f, sf::Color::White, 1);

    {
        gameFrameUpdate(registry, 0.f, worldSize);
        const std::optional<GameEventGameResult> optGameResult = tryGetGameResult(registry, playersCount);
        assert(!optGameResult.has_value());
    }

    std::optional<GameEventGameResult> optGameResult;

    for (int i = 0; i < 20; ++i)
    {
        gameFrameUpdate(registry, 0.1f, worldSize);
        optGameResult = tryGetGameResult(registry, playersCount);

        if (optGameResult)
        {
            break;
        }
    }

    assert(optGameResult.has_value());
    assert(!optGameResult->isTie());
    assert(optGameResult->victoriousPlayerIndex == 0);
    assert(registry.valid(ship1));
    assert(!registry.valid(ship2));
}

void runTests()
{
    // math tests
    testFloatWrap();
    testIsPointInsideCircle();
    testIsPointOnSegment();
    testIsSegmentIntersectCircle();
    testIsCircleIntersectCircle();
    testColorLerp();

    // game simulation tests
    testProjectileKillsShip();
    testShipsKillEachOtherWithProjectiles();
    testShipShipCollisionKillsBoth();
    testPlayerWinsGameWithKill();
}
