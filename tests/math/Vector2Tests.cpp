#include "math/Vector2.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Vector2 default is zero") {
    const Vector2 v;
    REQUIRE(v.x == 0.0f);
    REQUIRE(v.y == 0.0f);
}

TEST_CASE("Vector2 add sums components") {
    const Vector2 sum = add(Vector2{1.5f, -2.0f}, Vector2{0.5f, 3.0f});
    REQUIRE(sum.x == Catch::Approx(2.0f));
    REQUIRE(sum.y == Catch::Approx(1.0f));
}

TEST_CASE("Vector2 sub subtracts components") {
    const Vector2 delta = sub(Vector2{1.5f, -2.0f}, Vector2{0.5f, 3.0f});
    REQUIRE(delta.x == Catch::Approx(1.0f));
    REQUIRE(delta.y == Catch::Approx(-5.0f));
}

TEST_CASE("Vector2 sub of a vector with itself is zero") {
    const Vector2 delta = sub(Vector2{4.0f, -1.0f}, Vector2{4.0f, -1.0f});
    REQUIRE(delta.x == Catch::Approx(0.0f));
    REQUIRE(delta.y == Catch::Approx(0.0f));
}

TEST_CASE("Vector2 length of (3,4) is 5") {
    REQUIRE(length(Vector2{3.0f, 4.0f}) == Catch::Approx(5.0f));
}

TEST_CASE("Vector2 scale multiplies both components") {
    const Vector2 scaled = scale(Vector2{3.0f, -2.0f}, 2.0f);
    REQUIRE(scaled.x == Catch::Approx(6.0f));
    REQUIRE(scaled.y == Catch::Approx(-4.0f));
}

TEST_CASE("Vector2 scale by zero is zero") {
    const Vector2 scaled = scale(Vector2{3.0f, -2.0f}, 0.0f);
    REQUIRE(scaled.x == Catch::Approx(0.0f));
    REQUIRE(scaled.y == Catch::Approx(0.0f));
}

TEST_CASE("Vector2 scale by negative flips direction") {
    const Vector2 scaled = scale(Vector2{3.0f, -2.0f}, -1.0f);
    REQUIRE(scaled.x == Catch::Approx(-3.0f));
    REQUIRE(scaled.y == Catch::Approx(2.0f));
}

TEST_CASE("Vector2 normalize of (3,4) is (0.6, 0.8)") {
    const Vector2 unit = normalize(Vector2{3.0f, 4.0f});
    REQUIRE(unit.x == Catch::Approx(0.6f));
    REQUIRE(unit.y == Catch::Approx(0.8f));
}

TEST_CASE("Vector2 normalize has length 1") {
    REQUIRE(length(normalize(Vector2{3.0f, 4.0f})) == Catch::Approx(1.0f));
}

TEST_CASE("Vector2 normalize of zero is zero") {
    const Vector2 unit = normalize(Vector2{});
    REQUIRE(unit.x == Catch::Approx(0.0f));
    REQUIRE(unit.y == Catch::Approx(0.0f));
}

TEST_CASE("Vector2 almostEqual is true for identical vectors") {
    REQUIRE(almostEqual(Vector2{1.0f, -2.0f}, Vector2{1.0f, -2.0f}));
}

TEST_CASE("Vector2 almostEqual is true within default epsilon") {
    REQUIRE(almostEqual(Vector2{1.0f, 1.0f}, Vector2{1.00005f, 0.99995f}));
}

TEST_CASE("Vector2 almostEqual is false when far apart") {
    REQUIRE_FALSE(almostEqual(Vector2{1.0f, 1.0f}, Vector2{2.0f, 1.0f}));
}

TEST_CASE("Vector2 almostEqual uses the given epsilon") {
    REQUIRE(almostEqual(Vector2{0.0f, 0.0f}, Vector2{0.5f, 0.0f}, 0.6f));
    REQUIRE_FALSE(almostEqual(Vector2{0.0f, 0.0f}, Vector2{0.5f, 0.0f}, 0.4f));
}

TEST_CASE("Vector2 distance from (0,0) to (3,4) is 5") {
    REQUIRE(distance(Vector2{0.0f, 0.0f}, Vector2{3.0f, 4.0f}) == Catch::Approx(5.0f));
}

TEST_CASE("Vector2 distance is the same in both directions") {
    REQUIRE(distance(Vector2{1.0f, 2.0f}, Vector2{4.0f, 6.0f}) ==
            Catch::Approx(distance(Vector2{4.0f, 6.0f}, Vector2{1.0f, 2.0f})));
}

TEST_CASE("Vector2 distance to itself is zero") {
    REQUIRE(distance(Vector2{3.0f, -1.0f}, Vector2{3.0f, -1.0f}) == Catch::Approx(0.0f));
}
