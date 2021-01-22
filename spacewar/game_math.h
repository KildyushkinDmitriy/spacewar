#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

using Vec2 = sf::Vector2f;

constexpr float PI = 3.14159265359f;

bool floatEq(float a, float b, float tolerance = 0.0001);
float floatWrap(float val, float max);
float floatLerp(float from, float to, float t);

float randomFloatRange(float min, float max);

float radToDeg(float rad);
float degToRad(float deg);

float vec2Dot(Vec2 a, Vec2 b);
float vec2LengthSq(Vec2 vec);
float vec2Length(Vec2 vec);
float vec2DistSq(Vec2 a, Vec2 b);
float vec2Dist(Vec2 a, Vec2 b);
Vec2 vec2Wrap(Vec2 val, Vec2 max);
Vec2 vec2AngleToDir(float angle);
float vec2DirToAngle(Vec2 dir);

bool isPointInsideCircle(Vec2 point, Vec2 circleCenter, float circleRadius);
bool isPointOnSegment(Vec2 point, Vec2 segmentP1, Vec2 segmentP2, float precision = 0.0001);
bool isSegmentIntersectCircle(Vec2 segmentP1, Vec2 segmentP2, Vec2 circleCenter, float circleRadius);
bool isCircleIntersectCircle(Vec2 circle1Center, float circle1Radius, Vec2 circle2Center, float circle2Radius);

sf::Color colorLerp(sf::Color from, sf::Color to, float t);

struct FloatRange
{
    float min = 0.f;
    float max = 0.f;

    float getRandom() const;
};

struct ColorRange
{
    sf::Color min = sf::Color::Black;
    sf::Color max = sf::Color::Black;

    sf::Color getRandom() const;
};

struct CooldownTimer
{
    float timeLeft = 0.f;
    float cooldownTotalTime = 0.f;

    explicit CooldownTimer(const float cooldownTotalTime) : cooldownTotalTime(cooldownTotalTime)
    {
    }

    bool updateAndGetWasUsed(float dt, bool useInput);
};
