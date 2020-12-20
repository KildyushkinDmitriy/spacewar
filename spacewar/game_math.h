#pragma once

#include <SFML/System/Vector2.hpp>
#include <cmath>

using Vec2 = sf::Vector2f;

constexpr float PI = 3.14159265359f;
constexpr float TAU = PI * 2.f; // full turn in radians

float radToDeg(float rad);

bool floatEq(float a, float b, float tolerance = 0.0001);
float floatWrap(float val, float max);
Vec2 vec2Wrap(Vec2 val, Vec2 max);

float vec2Dot(Vec2 a, Vec2 b);
float vec2LengthSq(Vec2 vec);
float vec2Length(Vec2 vec);
float vec2DistSq(Vec2 a, Vec2 b);
float vec2Dist(Vec2 a, Vec2 b);

bool isPointInsideCircle(Vec2 point, Vec2 circleCenter, float circleRadius);
bool isPointOnSegment(Vec2 point, Vec2 segmentP1, Vec2 segmentP2, float precision = 0.0001);
bool isSegmentIntersectCircle(Vec2 segmentP1, Vec2 segmentP2, Vec2 circleCenter, float circleRadius);
bool isCircleIntersectCircle(Vec2 circle1Center, float circle1Radius, Vec2 circle2Center, float circle2Radius);
