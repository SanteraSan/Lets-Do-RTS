#include "nav/Approach.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("approachOrder reaches the goal when every cell on the row is open") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    int x = -1;
    int y = -1;
    REQUIRE(approachOrder(grid, NavLayer::Ground, Vector2{0.2f, 1.2f}, Vector2{6.2f, 1.2f}, x, y));
    REQUIRE(x == 6);
    REQUIRE(y == 1);
}

TEST_CASE("approachOrder stops a ground unit on the cell before a cliff") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    for (int row = 0; row < grid.height; ++row) {
        REQUIRE(setHeight(grid, 3, row, 2));
    }
    int x = -1;
    int y = -1;
    REQUIRE(approachOrder(grid, NavLayer::Ground, Vector2{0.2f, 1.2f}, Vector2{6.2f, 1.2f}, x, y));
    REQUIRE(x == 2);
    REQUIRE(y == 1);
}

TEST_CASE("approachOrder stops a ground unit in front of a clicked cliff") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    REQUIRE(setHeight(grid, 3, 1, 2));
    int x = -1;
    int y = -1;
    REQUIRE(approachOrder(grid, NavLayer::Ground, Vector2{0.2f, 1.2f}, Vector2{3.2f, 1.2f}, x, y));
    REQUIRE(x == 2);
    REQUIRE(y == 1);
}

TEST_CASE("approachOrder lets ground cross a cliff by a ramp and a plateau") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 3));
    for (int row = 0; row < grid.height; ++row) {
        REQUIRE(setHeight(grid, 3, row, 2));
        REQUIRE(setPlateau(grid, 4, row, 1));
    }
    REQUIRE(setRamp(grid, 3, 1, 1));
    REQUIRE(setRamp(grid, 5, 1, 0));
    int x = -1;
    int y = -1;
    REQUIRE(approachOrder(grid, NavLayer::Ground, Vector2{0.2f, 1.2f}, Vector2{6.2f, 1.2f}, x, y));
    REQUIRE(x == 6);
    REQUIRE(y == 1);
}

TEST_CASE("approachOrder lets an air unit reach a cell past the cliff") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    for (int row = 0; row < grid.height; ++row) {
        REQUIRE(setHeight(grid, 3, row, 2));
    }
    int x = -1;
    int y = -1;
    REQUIRE(approachOrder(grid, NavLayer::Air, Vector2{0.2f, 1.2f}, Vector2{6.2f, 1.2f}, x, y));
    REQUIRE(x == 6);
    REQUIRE(y == 1);
}

TEST_CASE("approachOrder rejects a point outside the grid") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 4));
    int x = 0;
    int y = 0;
    REQUIRE_FALSE(approachOrder(grid, NavLayer::Ground, Vector2{-1.0f, 1.0f}, Vector2{1.0f, 1.0f}, x, y));
    REQUIRE_FALSE(approachOrder(grid, NavLayer::Ground, Vector2{1.0f, 1.0f}, Vector2{20.0f, 1.0f}, x, y));
}
