#include "render/StandLift.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("flat ground and water have no stand lift") {
    REQUIRE(surfaceLift(0, 4.0f) == Catch::Approx(0.0f));
    REQUIRE(surfaceLift(-1, 4.0f) == Catch::Approx(0.0f));
}

TEST_CASE("a plateau lifts a ground unit onto its surface") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 4));
    grid.cellSize = 4.0f;
    REQUIRE(setPlateau(grid, 1, 1, 1));
    const Vector2 onPlateau = cellCenter(grid, 1, 1);
    REQUIRE(standLift(grid, onPlateau, NavLayer::Ground) == Catch::Approx(2.0f));
    REQUIRE(standLift(grid, onPlateau, NavLayer::Air) == Catch::Approx(0.0f));
    REQUIRE(standLift(grid, onPlateau, NavLayer::Underwater) == Catch::Approx(0.0f));
}

TEST_CASE("a cliff stands twice as high as the plateau beside it") {
    REQUIRE(surfaceLift(2, 4.0f) == Catch::Approx(4.0f));
    REQUIRE(surfaceLift(1, 4.0f) == Catch::Approx(2.0f));
}
