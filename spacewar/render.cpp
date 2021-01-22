#include "render.h"
#include "game_visual.h"

#include <array>
#include <SFML/Graphics.hpp>

class CustomVerticesShape : public sf::Drawable, public sf::Transformable
{
public:
    CustomVerticesShape(const sf::Texture* texture, const sf::VertexArray& vertices)
        : m_vertices(vertices),
          m_texture(texture)
    {
    }

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.transform *= getTransform();
        states.texture = m_texture;
        target.draw(m_vertices, states);
    }

    const sf::VertexArray m_vertices;
    const sf::Texture* m_texture;
};

static void drawShipTexture9times(const Vec2 pos, const float rotation, const Vec2& worldSize, sf::Drawable& drawable, sf::Transformable& transformable, sf::RenderWindow& window)
{
    // sprite is pointing upwards, but with zero rotation it must be pointing right, so offset the rotation
    transformable.setRotation(rotation + 90.f);

    // draw obj 9 times for wrap around world case, hoping sfml will do the culling 
    {
        transformable.setPosition(pos);
        window.draw(drawable);

        transformable.setPosition(pos + Vec2{worldSize.x, 0.f});
        window.draw(drawable);

        transformable.setPosition(pos + Vec2{-worldSize.x, 0.f});
        window.draw(drawable);

        transformable.setPosition(pos + Vec2{0.f, worldSize.y});
        window.draw(drawable);

        transformable.setPosition(pos + Vec2{0.f, -worldSize.y});
        window.draw(drawable);

        transformable.setPosition(pos + worldSize);
        window.draw(drawable);

        transformable.setPosition(pos - worldSize);
        window.draw(drawable);

        transformable.setPosition(pos + Vec2{-worldSize.x, worldSize.y});
        window.draw(drawable);

        transformable.setPosition(pos + Vec2{worldSize.x, -worldSize.y});
        window.draw(drawable);
    }
}

void drawStarsSystem(const entt::registry& registry, sf::RenderWindow& window, const float time)
{
    const auto view = registry.view<const PositionComponent, const StarComponent>();
    sf::CircleShape starShape;

    for (auto [_, position, star] : view.each())
    {
        const float angle = floatWrap(time * star.brightnessPeriodsPerSec * 360.f, 360.f);
        const float brightnessT = std::sin(degToRad(angle)) / 2.f + 0.5f;

        const float brightness = floatLerp(star.brightnessRange.min, star.brightnessRange.max, brightnessT);
        const sf::Uint8 colorMagnitude = static_cast<sf::Uint8>(brightness * 255.f);
        const sf::Color color = sf::Color{colorMagnitude, colorMagnitude, colorMagnitude, 255};

        starShape.setPosition(position.vec);
        starShape.setFillColor(color);
        starShape.setRadius(star.radius);
        starShape.setOrigin(Vec2{starShape.getRadius(), starShape.getRadius()});
        window.draw(starShape);
    }
}

void drawGravityWellsSystem(const entt::registry& registry, sf::RenderWindow& window, const float time)
{
    const auto view = registry.view<const PositionComponent, const GravityWellComponent>();

    for (auto [_, position, gravityWell] : view.each())
    {
        sf::CircleShape gravityWellShape;
        gravityWellShape.setPosition(position.vec);

        const int count = 20;

        for (int i = 0; i < count; ++i)
        {
            const float angle = floatWrap(time * 30.f + i * 260.f / count, 360.f);
            const float radiusMultiplier = std::cos(degToRad(angle)) / 2.f + 0.5f;
            const float radius = 100.f * radiusMultiplier;

            gravityWellShape.setFillColor(colorLerp(sf::Color{0, 0, 0, 120}, sf::Color{0, 0, 0, 0}, radiusMultiplier));
            gravityWellShape.setRadius(radius);
            gravityWellShape.setOrigin(Vec2{gravityWellShape.getRadius(), gravityWellShape.getRadius()});
            window.draw(gravityWellShape);
        }
    }
}

void drawParticlesSystem(const entt::registry& registry, sf::RenderWindow& window)
{
    sf::CircleShape particleShape;
    const auto view = registry.view<const PositionComponent, const ParticleComponent, const DestroyTimerComponent>();

    for (auto [_, position, particle, destroyTimer] : view.each())
    {
        const float t = (particle.totalLifetime - destroyTimer.timeLeft) / particle.totalLifetime;
        const float radius = floatLerp(particle.startRadius, particle.finishRadius, t);
        const sf::Color color = colorLerp(particle.startColor, particle.finishColor, t);

        particleShape.setRadius(radius);
        particleShape.setOrigin(Vec2{particleShape.getRadius(), particleShape.getRadius()});
        particleShape.setPosition(position.vec);
        particleShape.setFillColor(color);
        window.draw(particleShape);
    }
}

void drawUsingShipTextureSystem(const entt::registry& registry, sf::RenderWindow& window, const sf::Texture& shipTexture, const Vec2 worldSize)
{
    sf::RectangleShape shipShape;
    shipShape.setTexture(&shipTexture);

    const auto view = registry.view<const PositionComponent, const RotationComponent, const DrawUsingShipTextureComponent>();

    for (auto [entity, pos, rotation, draw] : view.each())
    {
        // somehow exclude does not work on view
        if (registry.try_get<DeadShipPieceComponent>(entity) != nullptr)
        {
            continue;
        }

        shipShape.setSize(draw.size);
        shipShape.setOrigin(draw.size / 2.f);
        shipShape.setFillColor(draw.color);
        drawShipTexture9times(pos.vec, rotation.angle, worldSize, shipShape, shipShape, window);
    }
}

void drawDeadShipPiecesSystem(const entt::registry& registry, sf::RenderWindow& window, const sf::Texture& shipTexture, const Vec2 worldSize)
{
    std::array<sf::Vertex, 9> shipPiecesVertices;
    shipPiecesVertices[0].position = Vec2{0.5f, 0.5f};
    shipPiecesVertices[1].position = Vec2{0.0f, 0.0f};
    shipPiecesVertices[2].position = Vec2{0.5f, 0.0f};
    shipPiecesVertices[3].position = Vec2{1.0f, 0.f};
    shipPiecesVertices[4].position = Vec2{1.0f, 0.5f};
    shipPiecesVertices[5].position = Vec2{1.0f, 1.0f};
    shipPiecesVertices[6].position = Vec2{0.5f, 1.0f};
    shipPiecesVertices[7].position = Vec2{0.0f, 1.0f};
    shipPiecesVertices[8].position = Vec2{0.0f, 0.5f};

    const Vec2 shipTextureSize = Vec2{shipTexture.getSize()};
    for (sf::Vertex& vertex : shipPiecesVertices)
    {
        vertex.texCoords = vertex.position;
        vertex.texCoords.x *= shipTextureSize.x;
        vertex.texCoords.y *= shipTextureSize.y;
    }

    const auto view = registry.view<
        const PositionComponent,
        const RotationComponent,
        const DrawUsingShipTextureComponent,
        const DeadShipPieceComponent>();

    for (auto [entity, pos, rotation, draw, deadPiece] : view.each())
    {
        const int i = deadPiece.pieceIndex;
        sf::VertexArray triangleVertices{sf::Triangles, 3};
        triangleVertices[0] = shipPiecesVertices[0];
        triangleVertices[1] = shipPiecesVertices[i + 1];
        triangleVertices[2] = shipPiecesVertices[i == 7 ? 1 : i + 2];

        for (size_t vertexIdx = 0; vertexIdx < triangleVertices.getVertexCount(); ++vertexIdx)
        {
            triangleVertices[vertexIdx].color = draw.color;
        }

        CustomVerticesShape customShape{&shipTexture, triangleVertices};
        customShape.setOrigin(Vec2{0.5, 0.5});
        customShape.setScale(draw.size);

        drawShipTexture9times(pos.vec, rotation.angle, worldSize, customShape, customShape, window);
    }
}

void drawGame(sf::RenderWindow& window, const sf::Texture& shipTexture, const entt::registry& registry, const Vec2 worldSize, const float time)
{
    drawStarsSystem(registry, window, time);
    drawGravityWellsSystem(registry, window, time);
    drawParticlesSystem(registry, window);
    drawUsingShipTextureSystem(registry, window, shipTexture, worldSize);
    drawDeadShipPiecesSystem(registry, window, shipTexture, worldSize);
}


// static void drawThickLine(const Vec2 a, const Vec2 b, const float thickness, const sf::Color color,
//                           sf::RenderWindow& window)
// {
//     const Vec2 diff = b - a;
//     const float angle = vec2DirToAngle(diff);
//
//     sf::RectangleShape rectShape{Vec2{vec2Length(diff), thickness}};
//     rectShape.setPosition(a);
//     rectShape.setOrigin(Vec2{0.f, thickness / 2.f});
//     rectShape.rotate(angle);
//     rectShape.setFillColor(color);
//     window.draw(rectShape);
// }
//
//
// void renderGameDebug(const GameWorld& world, sf::RenderWindow& window, const sf::Font& font)
// {
//     // gravitation field visualize
//     for (const GravityWell& well : world.gravityWells)
//     {
//         const int gradationsCount = 20;
//         const float radiusIncrement = well.maxRadius / (gradationsCount - 1);
//         const Vec2 dir = vec2AngleToDir(180.f + 45.f);
//
//         struct Measure
//         {
//             Vec2 point{};
//             float radius = 0.f;
//             float gravityPower = 0.f;
//         };
//
//         std::vector<Measure> measures;
//
//         for (int i = gradationsCount - 1; i >= 0; --i)
//         {
//             const float radius = i * radiusIncrement;
//             const Vec2 point = well.pos + dir * radius;
//             const float gravityPower = gameGetGravityWellPowerAtRadius(well, radius);
//             measures.push_back({point, radius, gravityPower});
//         }
//
//         sf::CircleShape gravityCircleShape;
//         gravityCircleShape.setPosition(well.pos);
//
//         for (const Measure& measure : measures)
//         {
//             gravityCircleShape.setRadius(measure.radius);
//             gravityCircleShape.setOrigin(Vec2{gravityCircleShape.getRadius(), gravityCircleShape.getRadius()});
//
//             sf::Uint8 grayscale = static_cast<sf::Uint8>(measure.gravityPower / well.maxPower * 255.f);
//             gravityCircleShape.setFillColor(sf::Color{grayscale, grayscale, grayscale});
//
//             window.draw(gravityCircleShape);
//         }
//
//         sf::CircleShape pointShape{5.f};
//         pointShape.setOrigin(Vec2{pointShape.getRadius(), pointShape.getRadius()});
//         pointShape.setFillColor(sf::Color::Magenta);
//
//         sf::Text gravityPowerText;
//         gravityPowerText.setFont(font);
//         gravityPowerText.setCharacterSize(20);
//         gravityPowerText.setFillColor(sf::Color::Magenta);
//
//         for (const Measure& measure : measures)
//         {
//             pointShape.setPosition(measure.point);
//             window.draw(pointShape);
//
//             gravityPowerText.setString(std::to_string(measure.gravityPower));
//             gravityPowerText.setPosition(measure.point + Vec2{10.f, -10.f});
//             window.draw(gravityPowerText);
//         }
//
//         sf::CircleShape dragCircleShape{well.dragRadius};
//         dragCircleShape.setOrigin(Vec2{dragCircleShape.getRadius(), dragCircleShape.getRadius()});
//         dragCircleShape.setPosition(well.pos);
//         dragCircleShape.setFillColor(sf::Color::Transparent);
//         dragCircleShape.setOutlineColor(sf::Color::Magenta);
//         dragCircleShape.setOutlineThickness(5.f);
//         window.draw(dragCircleShape);
//     }
//
//     for (const Ship& ship : world.ships)
//     {
//         {
//             // ship collision body
//             sf::CircleShape shipCollisionShape{world.settings.shipCollisionRadius};
//             shipCollisionShape.setOrigin(Vec2{shipCollisionShape.getRadius(), shipCollisionShape.getRadius()});
//             shipCollisionShape.setFillColor(ship.isDead ? sf::Color::Blue : sf::Color::Green);
//             shipCollisionShape.setPosition(ship.pos);
//             window.draw(shipCollisionShape);
//         }
//
//         // velocity
//         drawThickLine(ship.pos, ship.pos + ship.velocity, 5.f, sf::Color::Yellow, window);
//
//         // gravity
//         for (const GravityWell& well : world.gravityWells)
//         {
//             Vec2 gravityPower = gameGetGravityWellVectorAtPoint(well, ship.pos);
//             drawThickLine(ship.pos, ship.pos + gravityPower, 5.f, sf::Color::Magenta, window);
//         }
//
//         // input acceleration
//         if (ship.input.thrust)
//         {
//             Vec2 accel = vec2AngleToDir(ship.rotation) * world.settings.shipThrustAcceleration;
//             drawThickLine(ship.pos, ship.pos + accel, 5.f, sf::Color::Green, window);
//         }
//
//         {
//             // ship direction
//             sf::RectangleShape rectShape{Vec2{world.settings.shipCollisionRadius, 5.f}};
//             rectShape.setPosition(ship.pos);
//             rectShape.setOrigin(Vec2{0.f, rectShape.getSize().y / 2.f});
//             rectShape.rotate(ship.rotation);
//             rectShape.setFillColor(sf::Color::Black);
//             window.draw(rectShape);
//         }
//     }
//
//     {
//         // projectiles
//         sf::CircleShape projectileShape{5.f};
//         projectileShape.setOrigin(Vec2{projectileShape.getRadius(), projectileShape.getRadius()});
//         projectileShape.setFillColor(sf::Color::Red);
//
//         for (const Projectile& projectile : world.projectiles)
//         {
//             projectileShape.setPosition(projectile.pos);
//             window.draw(projectileShape);
//         }
//     }
// }
