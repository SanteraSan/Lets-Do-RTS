#include "render/GroundSprite.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

TEST_CASE("ground sprite feet sit on the stand point") {
    const Camera camera{};
    const Vector2 position{4.0f, 6.0f};
    const SpriteScreen frame = groundSpriteScreen(position, Vector2{0.0f, 1.0f}, 2.0f, 4.0f, 10, 20, camera);
    const ScreenPoint feet = worldToScreen(position, 2.0f, camera);
    REQUIRE(frame.feet.x == feet.x);
    REQUIRE(frame.feet.y == feet.y);
}

TEST_CASE("ground sprite stands a cell and a half tall") {
    const Camera camera{};
    const SpriteScreen frame = groundSpriteScreen(Vector2{0.0f, 0.0f}, Vector2{0.0f, 1.0f}, 0.0f, 4.0f, 10, 20, camera);
    const float edge = std::sqrt(1.0f + camera.yScale * camera.yScale);
    const int expected = static_cast<int>(4.0f * kInfantryCellsTall * camera.pixelsPerUnit * edge);
    REQUIRE(frame.pixelHeight == expected);
    REQUIRE(frame.pixelWidth == static_cast<int>(static_cast<float>(expected) * 10.0f / 20.0f));
}

TEST_CASE("a still soldier keeps his feet on the stand") {
    const Camera camera{};
    const Vector2 position{4.0f, 6.0f};
    const float bob = walkBob(kWalkStride * 0.5f, false);
    const SpriteScreen frame = groundSpriteScreen(position, Vector2{0.0f, 1.0f}, 2.0f, 4.0f, 10, 20, camera, bob);
    const ScreenPoint feet = worldToScreen(position, 2.0f, camera);
    REQUIRE(bob == 0.0f);
    REQUIRE(frame.feet.y == feet.y);
}

TEST_CASE("a walking soldier lifts at mid stride and plants his feet on the next step") {
    const Camera camera{};
    const Vector2 position{0.0f, 0.0f};
    const float mid = walkBob(kWalkStride * 0.5f, true);
    const float planted = walkBob(kWalkStride, true);
    const SpriteScreen lifted =
        groundSpriteScreen(position, Vector2{1.0f, 0.0f}, 0.0f, 4.0f, 10, 20, camera, mid);
    const SpriteScreen down =
        groundSpriteScreen(position, Vector2{1.0f, 0.0f}, 0.0f, 4.0f, 10, 20, camera, planted);
    REQUIRE(mid == Catch::Approx(kWalkBob));
    REQUIRE(planted == Catch::Approx(0.0f).margin(0.001f));
    REQUIRE(lifted.feet.y < down.feet.y);
}

TEST_CASE("ground sprite flips when facing screen right") {
    const Camera camera{};
    const SpriteScreen east = groundSpriteScreen(Vector2{0.0f, 0.0f}, Vector2{1.0f, 0.0f}, 0.0f, 4.0f, 10, 20, camera);
    const SpriteScreen south = groundSpriteScreen(Vector2{0.0f, 0.0f}, Vector2{0.0f, 1.0f}, 0.0f, 4.0f, 10, 20, camera);
    REQUIRE(east.flip);
    REQUIRE_FALSE(south.flip);
}
