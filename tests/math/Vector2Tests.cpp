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

TEST_CASE("Vector2 length of (3,4) is 5") {
    REQUIRE(length(Vector2{3.0f, 4.0f}) == Catch::Approx(5.0f));
}
