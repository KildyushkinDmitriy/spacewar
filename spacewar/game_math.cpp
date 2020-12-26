#include "game_math.h"

#include <cmath>

float radToDeg(const float rad)
{
    return rad / PI * 180.f;
}


float degToRad(const float deg)
{
    return deg / 180.f * PI;
}

bool floatEq(const float a, const float b, const float tolerance)
{
    return std::fabs(a - b) <= tolerance;
}

float floatWrap(const float val, const float max)
{
    const int whole = static_cast<int>(val / max);
    const float remainder = val - whole * max;
    if (val >= 0.f)
    {
        return remainder;
    }
    return max + remainder;
}

Vec2 vec2Wrap(const Vec2 val, const Vec2 max)
{
    return Vec2{floatWrap(val.x, max.x), floatWrap(val.y, max.y)};
}

float vec2Dot(const Vec2 a, const Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

float vec2LengthSq(const Vec2 vec)
{
    return vec2Dot(vec, vec);
}

float vec2Length(const Vec2 vec)
{
    return std::sqrt(vec2LengthSq(vec));
}

float vec2DistSq(const Vec2 a, const Vec2 b)
{
    return vec2LengthSq(a - b);
}

float vec2Dist(const Vec2 a, const Vec2 b)
{
    return std::sqrt(vec2DistSq(a, b));
}

Vec2 vec2RotationToDir(const float rotation)
{
    const float rad = degToRad(rotation);
    return Vec2{cos(rad), sin(rad)};
}

bool isPointInsideCircle(const Vec2 point, const Vec2 circleCenter, const float circleRadius)
{
    return vec2DistSq(point, circleCenter) <= circleRadius * circleRadius;
}

bool isPointOnSegment(const Vec2 point, const Vec2 segmentP1, const Vec2 segmentP2, const float precision)
{
    const float length = vec2Dist(segmentP1, segmentP2);
    const float dist1 = vec2Dist(segmentP1, point);
    const float dist2 = vec2Dist(segmentP2, point);

    return floatEq(length, dist1 + dist2, precision);
}

bool isSegmentIntersectCircle(const Vec2 segmentP1, const Vec2 segmentP2, const Vec2 circleCenter,
                              const float circleRadius)
{
    if (isPointInsideCircle(segmentP1, circleCenter, circleRadius))
    {
        return true;
    }

    if (isPointInsideCircle(segmentP2, circleCenter, circleRadius))
    {
        return true;
    }

    const float lineLengthSq = vec2DistSq(segmentP1, segmentP2);

    const float dot = (
        (circleCenter.x - segmentP1.x) * (segmentP2.x - segmentP1.x)
        + (circleCenter.y - segmentP1.y) * (segmentP2.y - segmentP1.y)
    ) / lineLengthSq;

    const Vec2 closest = segmentP1 + dot * (segmentP2 - segmentP1);

    if (!isPointOnSegment(closest, segmentP1, segmentP2))
    {
        return false;
    }

    const float distClosestToCircleOriginSq = vec2DistSq(closest, circleCenter);
    return distClosestToCircleOriginSq <= circleRadius * circleRadius;
}

bool isCircleIntersectCircle(const Vec2 circle1Center, const float circle1Radius, const Vec2 circle2Center,
                             const float circle2Radius)
{
    const float radiusSum = circle1Radius + circle2Radius;
    return vec2DistSq(circle1Center, circle2Center) <= radiusSum * radiusSum;
}
