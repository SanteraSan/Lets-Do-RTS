#include "platform/Camera.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("worldToScreen maps origin to the camera offset") {
    const Camera camera{};
    const ScreenPoint point = worldToScreen(Vector2{0.0f, 0.0f}, camera);
    REQUIRE(point.x == 640);
    REQUIRE(point.y == 180);
}

TEST_CASE("a raised point keeps its screen x and moves up") {
    const Camera camera{};
    const ScreenPoint flat = worldToScreen(Vector2{0.0f, 0.0f}, camera);
    const ScreenPoint raised = worldToScreen(Vector2{0.0f, 0.0f}, 2.0f, camera);
    REQUIRE(raised.x == flat.x);
    REQUIRE(raised.y < flat.y);
}

TEST_CASE("worldToScreen moves +X right and down") {
    const Camera camera{};
    const ScreenPoint point = worldToScreen(Vector2{60.0f, 0.0f}, camera);
    REQUIRE(point.x == 1240);
    REQUIRE(point.y == 480);
}

TEST_CASE("screenToWorld inverts worldToScreen at the origin") {
    const Camera camera{};
    const Vector2 world = screenToWorld(ScreenPoint{640, 180}, camera);
    REQUIRE(world.x == Catch::Approx(0.0f));
    REQUIRE(world.y == Catch::Approx(0.0f));
}

TEST_CASE("screenToWorld inverts worldToScreen along X") {
    const Camera camera{};
    const Vector2 world = screenToWorld(ScreenPoint{1240, 480}, camera);
    REQUIRE(world.x == Catch::Approx(60.0f));
    REQUIRE(world.y == Catch::Approx(0.0f));
}

TEST_CASE("screenToWorld inverts a point with both axes") {
    const Camera camera{};
    const Vector2 world = screenToWorld(worldToScreen(Vector2{4.0f, 12.0f}, camera), camera);
    REQUIRE(world.x == Catch::Approx(4.0f));
    REQUIRE(world.y == Catch::Approx(12.0f));
}

TEST_CASE("screenToWorld rejects a non-positive pixelsPerUnit") {
    Camera camera;
    camera.pixelsPerUnit = 0.0f;
    Vector2 world = screenToWorld(ScreenPoint{200, 360}, camera);
    REQUIRE(world.x == Catch::Approx(0.0f));
    REQUIRE(world.y == Catch::Approx(0.0f));
    camera.pixelsPerUnit = -10.0f;
    world = screenToWorld(ScreenPoint{200, 360}, camera);
    REQUIRE(world.x == Catch::Approx(0.0f));
    REQUIRE(world.y == Catch::Approx(0.0f));
}

TEST_CASE("screenToWorld rejects a non-positive yScale") {
    Camera camera;
    camera.yScale = 0.0f;
    const Vector2 world = screenToWorld(ScreenPoint{640, 180}, camera);
    REQUIRE(world.x == Catch::Approx(0.0f));
    REQUIRE(world.y == Catch::Approx(0.0f));
}

TEST_CASE("panCameraPixels shifts the origin and stays invertible") {
    Camera camera{};
    const Vector2 before = camera.origin;
    panCameraPixels(camera, 10, 0);
    REQUIRE(camera.origin.x == Catch::Approx(before.x + 0.5f));
    REQUIRE(camera.origin.y == Catch::Approx(before.y - 0.5f));
    const Vector2 world = screenToWorld(worldToScreen(Vector2{4.0f, 8.0f}, camera), camera);
    REQUIRE(world.x == Catch::Approx(4.0f));
    REQUIRE(world.y == Catch::Approx(8.0f));
}

TEST_CASE("panCameraPixels ignores a non-positive scale") {
    Camera camera{};
    camera.pixelsPerUnit = 0.0f;
    panCameraPixels(camera, 10, 0);
    REQUIRE(camera.origin.x == Catch::Approx(0.0f));
    REQUIRE(camera.origin.y == Catch::Approx(0.0f));
}

TEST_CASE("arrow directions pan by the given pixel step") {
    Camera camera{};
    panCameraDirections(camera, false, true, false, false, 10);
    REQUIRE(camera.origin.x == Catch::Approx(0.5f));
    REQUIRE(camera.origin.y == Catch::Approx(-0.5f));
}

TEST_CASE("middle drag follows the cursor and the first press does not jump") {
    Camera camera{};
    CameraDrag drag;
    dragCamera(camera, drag, true, ScreenPoint{100, 100});
    REQUIRE(camera.origin.x == Catch::Approx(0.0f));
    dragCamera(camera, drag, true, ScreenPoint{110, 100});
    REQUIRE(camera.origin.x == Catch::Approx(-0.5f));
    REQUIRE(camera.origin.y == Catch::Approx(0.5f));
    dragCamera(camera, drag, false, ScreenPoint{110, 100});
    REQUIRE_FALSE(drag.active);
}

TEST_CASE("worldLengthToPixels scales a positive radius") {
    REQUIRE(worldLengthToPixels(2.0f, Camera{}) == 20);
}

TEST_CASE("worldLengthToPixels rejects a non-positive length") {
    REQUIRE(worldLengthToPixels(0.0f, Camera{}) == 0);
    REQUIRE(worldLengthToPixels(-1.0f, Camera{}) == 0);
}
