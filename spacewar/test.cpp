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

void runTests()
{
    testFloatWrap();
    testIsPointInsideCircle();
    testIsPointOnSegment();
    testIsSegmentIntersectCircle();
}
