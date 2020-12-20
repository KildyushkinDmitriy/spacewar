#include <assert.h>

#include "game.h"

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

    const float angle = TAU / 8.f;
    const Vec2 point = Vec2{std::cos(angle), std::sin(angle)} * 3.f;

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

static void testProjectileKillsShip()
{
    GameWorld world{};
    world.size = Vec2{100.f, 100.f};
    world.settings.shipCollisionRadius = 5.f;

    {
        Ship ship{};
        ship.pos = Vec2{10.f, 10.f};
        world.ships.push_back(ship);
    }

    {
        Projectile projectile{};
        projectile.lifetimeLeft = 100.f;
        projectile.pos = Vec2{20.f, 14.f};
        projectile.velocity = Vec2{-10.f, 0.f};
        world.projectiles.push_back(projectile);
    }

    assert(!world.ships[0].isDead);
    assert(world.projectiles.size() == 1);

    gameUpdate(world, 10.f);

    assert(world.ships[0].isDead);
    assert(world.projectiles.size() == 0);
}

static void testShipsKillEachOtherWithProjectiles()
{
    GameWorld world{};
    world.size = Vec2{100.f, 100.f};
    world.settings.shipCollisionRadius = 0.5f;
    world.settings.projectileLifetime = 10000.f;
    world.settings.projectileSpeed = 10.f;
    world.settings.shootCooldown = 2.0f;
    world.settings.muzzleExtraOffset = 0.1;

    {
        Ship ship{};
        ship.pos = Vec2{10.f, 10.f};
        ship.input.shoot = true;
        world.ships.push_back(ship);
    }

    {
        Ship ship{};
        ship.pos = Vec2{20.f, 10.f};
        ship.rotation = TAU / 2.f;
        ship.input.shoot = true;
        world.ships.push_back(ship);
    }

    assert(!world.ships[0].isDead);
    assert(!world.ships[1].isDead);

    gameUpdate(world, 0.5f);

    assert(!world.ships[0].isDead);
    assert(!world.ships[1].isDead);

    gameUpdate(world, 0.5f);

    assert(world.ships[0].isDead);
    assert(world.ships[1].isDead);
}

void testProjectileLifetime()
{
    GameWorld world{};
    world.size = Vec2{100.f, 100.f};
    {
        Projectile projectile{};
        projectile.lifetimeLeft = 1.f;
        world.projectiles.push_back(projectile);
    }

    assert(world.projectiles.size() == 1);
    gameUpdate(world, 0.9f);
    assert(world.projectiles.size() == 1);
    gameUpdate(world, 0.2f);
    assert(world.projectiles.size() == 0);
}

static void testShipShipCollisionKillsBoth()
{
    GameWorld world{};
    world.size = Vec2{100.f, 100.f};
    world.settings.shipCollisionRadius = 0.5f;

    {
        Ship ship{};
        ship.pos = Vec2{10.f, 10.f};
        ship.velocity = Vec2{5.f, 0.f};
        world.ships.push_back(ship);
    }

    {
        Ship ship{};
        ship.pos = Vec2{20.f, 10.f};
        ship.velocity = Vec2{-5.f, 0.f};
        world.ships.push_back(ship);
    }

    assert(!world.ships[0].isDead);
    assert(!world.ships[1].isDead);

    gameUpdate(world, 0.5f);

    assert(!world.ships[0].isDead);
    assert(!world.ships[1].isDead);

    gameUpdate(world, 0.5f);

    assert(world.ships[0].isDead);
    assert(world.ships[1].isDead);
}

void runTests()
{
    testFloatWrap();
    testIsPointInsideCircle();
    testIsPointOnSegment();
    testIsSegmentIntersectCircle();
    testIsCircleIntersectCircle();

    testProjectileKillsShip();
    testShipsKillEachOtherWithProjectiles();
    testProjectileLifetime();
    testShipShipCollisionKillsBoth();
}
