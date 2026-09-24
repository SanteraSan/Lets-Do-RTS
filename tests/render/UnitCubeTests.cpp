#include "render/UnitCube.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

const CubeFace* frontFace(const UnitCube& cube) {
    for (int i = 0; i < cube.count; ++i) {
        if (cube.faces[i].front) {
            return &cube.faces[i];
        }
    }
    return nullptr;
}

const CubeFace* topFace(const UnitCube& cube) {
    for (int i = 0; i < cube.count; ++i) {
        if (cube.faces[i].top) {
            return &cube.faces[i];
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("cube front sits ahead of the center when facing east") {
    const UnitCube cube = visibleUnitCube(Vector2{0.0f, 0.0f}, Vector2{1.0f, 0.0f}, 2.0f, 0.5f);
    const CubeFace* front = frontFace(cube);
    REQUIRE(front != nullptr);
    const Vector2 center = faceCenter(*front);
    REQUIRE(center.x > 0.0f);
    REQUIRE(center.y == Catch::Approx(0.0f));
}

TEST_CASE("cube front follows a north facing") {
    const UnitCube cube = visibleUnitCube(Vector2{4.0f, 6.0f}, Vector2{0.0f, 1.0f}, 2.0f, 0.5f);
    const CubeFace* front = frontFace(cube);
    REQUIRE(front != nullptr);
    const Vector2 center = faceCenter(*front);
    REQUIRE(center.x == Catch::Approx(4.0f));
    REQUIRE(center.y > 6.0f);
}

TEST_CASE("cube top rests a full side above the ground") {
    const UnitCube cube = visibleUnitCube(Vector2{0.0f, 0.0f}, Vector2{1.0f, 0.0f}, 2.0f, 0.5f);
    const CubeFace* top = topFace(cube);
    REQUIRE(top != nullptr);
    for (const RaisedPoint& corner : top->corner) {
        REQUIRE(corner.height == Catch::Approx(2.0f));
    }
    REQUIRE(cube.faces[0].corner[0].height == Catch::Approx(0.0f));
}

TEST_CASE("a zero facing still shows a front to the east") {
    const UnitCube cube = visibleUnitCube(Vector2{0.0f, 0.0f}, Vector2{0.0f, 0.0f}, 2.0f, 0.5f);
    const CubeFace* front = frontFace(cube);
    REQUIRE(front != nullptr);
    REQUIRE(faceCenter(*front).x > 0.0f);
}

TEST_CASE("a non-positive side draws no cube faces") {
    const UnitCube cube = visibleUnitCube(Vector2{0.0f, 0.0f}, Vector2{1.0f, 0.0f}, 0.0f, 0.5f);
    REQUIRE(cube.count == 0);
}
