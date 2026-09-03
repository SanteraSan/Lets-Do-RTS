#include "math/MoveTowards.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("moveTowards walks along X at speed times dt") {
    Vector2 position{0.0f, 0.0f};
    moveTowards(position, Vector2{10.0f, 0.0f}, 2.0f, 1.0f);
    REQUIRE(position.x == Catch::Approx(2.0f));
    REQUIRE(position.y == Catch::Approx(0.0f));
}

TEST_CASE("moveTowards two half steps match one full step") {
    Vector2 once{0.0f, 0.0f};
    moveTowards(once, Vector2{10.0f, 0.0f}, 2.0f, 1.0f);

    Vector2 twice{0.0f, 0.0f};
    moveTowards(twice, Vector2{10.0f, 0.0f}, 2.0f, 0.5f);
    moveTowards(twice, Vector2{10.0f, 0.0f}, 2.0f, 0.5f);

    REQUIRE(almostEqual(once, twice));
}

TEST_CASE("moveTowards snaps when the step would overshoot") {
    Vector2 position{9.5f, 0.0f};
    moveTowards(position, Vector2{10.0f, 0.0f}, 2.0f, 1.0f);
    REQUIRE(almostEqual(position, Vector2{10.0f, 0.0f}));
}

TEST_CASE("moveTowards stays put when already at the target") {
    Vector2 position{4.0f, -1.0f};
    moveTowards(position, Vector2{4.0f, -1.0f}, 5.0f, 1.0f);
    REQUIRE(almostEqual(position, Vector2{4.0f, -1.0f}));
}

TEST_CASE("moveTowards ignores non-positive speed or dt") {
    Vector2 position{0.0f, 0.0f};
    moveTowards(position, Vector2{10.0f, 0.0f}, 0.0f, 1.0f);
    moveTowards(position, Vector2{10.0f, 0.0f}, 2.0f, -1.0f);
    REQUIRE(almostEqual(position, Vector2{0.0f, 0.0f}));
}
