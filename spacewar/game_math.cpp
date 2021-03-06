﻿#include "game_math.h"

#include <cmath>
#include <random>

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

Vec2 vec2AngleToDir(const float angle)
{
    const float rad = degToRad(floatWrap(angle, 360.f));
    return Vec2{cos(rad), sin(rad)};
}

float vec2DirToAngle(const Vec2 dir)
{
    const float rad = std::atan2f(dir.y, dir.x);
    return floatWrap(radToDeg(rad), 360.f);
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

float floatLerp(const float from, const float to, const float t)
{
    return from + t * (to - from);
}

sf::Color colorLerp(const sf::Color from, const sf::Color to, const float t)
{
    sf::Color result;
    result.r = static_cast<sf::Uint8>(floatLerp(from.r, to.r, t));
    result.g = static_cast<sf::Uint8>(floatLerp(from.g, to.g, t));
    result.b = static_cast<sf::Uint8>(floatLerp(from.b, to.b, t));
    result.a = static_cast<sf::Uint8>(floatLerp(from.a, to.a, t));
    return result;
}

std::default_random_engine rndEngine{std::random_device{}()};

float randomFloatRange(const float min, const float max)
{
    const std::uniform_real_distribution<float> distribution{min, max};
    return distribution(rndEngine);
}

float FloatRange::getRandom() const
{
    return randomFloatRange(min, max);
}

sf::Color ColorRange::getRandom() const
{
    return colorLerp(min, max, randomFloatRange(0.f, 1.f));
}

bool CooldownTimer::updateAndGetWasUsed(const float dt, const bool useInput)
{
    timeLeft -= dt;
    if (timeLeft > 0)
    {
        return false;
    }
    timeLeft = 0.f;

    if (useInput)
    {
        timeLeft = cooldownTotalTime;
        return true;
    }

    return false;
}
