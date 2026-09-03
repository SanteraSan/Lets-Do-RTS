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
