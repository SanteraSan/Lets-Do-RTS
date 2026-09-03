#include "platform/Camera.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("worldToScreen maps origin to the camera offset") {
    const Camera camera{};
    const ScreenPoint point = worldToScreen(Vector2{0.0f, 0.0f}, camera);
    REQUIRE(point.x == 200);
    REQUIRE(point.y == 360);
}

TEST_CASE("worldToScreen scales world X by pixelsPerUnit") {
    const Camera camera{};
    const ScreenPoint point = worldToScreen(Vector2{60.0f, 0.0f}, camera);
    REQUIRE(point.x == 800);
    REQUIRE(point.y == 360);
}

TEST_CASE("worldLengthToPixels scales a positive radius") {
    REQUIRE(worldLengthToPixels(2.0f, Camera{}) == 20);
}

TEST_CASE("worldLengthToPixels rejects a non-positive length") {
    REQUIRE(worldLengthToPixels(0.0f, Camera{}) == 0);
    REQUIRE(worldLengthToPixels(-1.0f, Camera{}) == 0);
}
