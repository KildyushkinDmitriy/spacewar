#include "game_math.h"

float radToDeg(const float rad)
{
    return rad / PI * 180.f;
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

bool isPointInsideCircle(const Vec2 point, const Vec2 circleOrigin, const float circleRadius)
{
    const Vec2 diff = point - circleOrigin;
    return vec2LengthSq(diff) <= circleRadius * circleRadius;
}

bool isPointOnSegment(const Vec2 point, const Vec2 segmentP1, const Vec2 segmentP2, const float precision)
{
    const float length = vec2Dist(segmentP1, segmentP2);
    const float dist1 = vec2Dist(segmentP1, point);
    const float dist2 = vec2Dist(segmentP2, point);

    return floatEq(length, dist1 + dist2, precision);
}

bool isSegmentIntersectCircle(const Vec2 segmentP1, const Vec2 segmentP2, const Vec2 circleOrigin,
                              const float circleRadius)
{
    if (isPointInsideCircle(segmentP1, circleOrigin, circleRadius))
    {
        return true;
    }

    if (isPointInsideCircle(segmentP2, circleOrigin, circleRadius))
    {
        return true;
    }

    const float lineLengthSq = vec2DistSq(segmentP1, segmentP2);

    const float dot = (
        (circleOrigin.x - segmentP1.x) * (segmentP2.x - segmentP1.x)
        + (circleOrigin.y - segmentP1.y) * (segmentP2.y - segmentP1.y)
    ) / lineLengthSq;

    const float closestX = segmentP1.x + dot * (segmentP2.x - segmentP1.x);
    const float closestY = segmentP1.y + dot * (segmentP2.y - segmentP1.y);
    const Vec2 closest{closestX, closestY};

    const bool onSegment = isPointOnSegment(closest, segmentP1, segmentP2);
    if (!onSegment)
    {
        return false;
    }

    const float distClosestToCircleOriginSq = vec2DistSq(closest, circleOrigin);

    if (distClosestToCircleOriginSq <= circleRadius * circleRadius)
    {
        return true;
    }

    return false;
}
