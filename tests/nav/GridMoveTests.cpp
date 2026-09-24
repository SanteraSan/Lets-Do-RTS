#include "nav/GridMove.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("moveOnGrid keeps a short step inside the current cell") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    Vector2 position{0.2f, 1.2f};
    moveOnGrid(position, Vector2{0.5f, 1.2f}, NavLayer::Ground, grid, 1.0f, 0.1f);
    REQUIRE(position.x == Catch::Approx(0.3f));
    REQUIRE(position.y == Catch::Approx(1.2f));
}

TEST_CASE("moveOnGrid advances at most one cell in a tick") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    Vector2 position{0.2f, 1.2f};
    moveOnGrid(position, Vector2{6.2f, 1.2f}, NavLayer::Ground, grid, 10.0f, 1.0f);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, position, cellX, cellY));
    REQUIRE(cellX == 1);
    REQUIRE(cellY == 1);
}

TEST_CASE("moveOnGrid keeps a ground unit out of a cliff cell") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    REQUIRE(setHeight(grid, 2, 1, 3));
    Vector2 position{1.4f, 1.2f};
    moveOnGrid(position, Vector2{6.2f, 1.2f}, NavLayer::Ground, grid, 10.0f, 1.0f);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, position, cellX, cellY));
    REQUIRE(cellX == 1);
    REQUIRE(cellY == 1);
}

TEST_CASE("moveOnGrid lets an air unit enter a cliff cell") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    REQUIRE(setHeight(grid, 2, 1, 3));
    Vector2 position{1.4f, 1.2f};
    moveOnGrid(position, Vector2{6.2f, 1.2f}, NavLayer::Air, grid, 10.0f, 1.0f);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, position, cellX, cellY));
    REQUIRE(cellX == 2);
    REQUIRE(cellY == 1);
}

TEST_CASE("moveOnGrid ignores a non-positive speed") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 4));
    Vector2 position{0.2f, 0.2f};
    moveOnGrid(position, Vector2{3.0f, 0.2f}, NavLayer::Ground, grid, 0.0f, 1.0f);
    REQUIRE(position.x == Catch::Approx(0.2f));
}
