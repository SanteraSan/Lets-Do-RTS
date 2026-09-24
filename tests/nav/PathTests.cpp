#include "nav/Path.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("findPath walks a clear row cell by cell") {
    Grid grid;
    REQUIRE(makeGrid(grid, 6, 3));
    const std::vector<PathCell> path = findPath(grid, NavLayer::Ground, 0, 1, 3, 1);
    REQUIRE(path.size() == 3);
    REQUIRE(path[0].x == 1);
    REQUIRE(path[0].y == 1);
    REQUIRE(path[2].x == 3);
    REQUIRE(path[2].y == 1);
}

TEST_CASE("findPath goes around a cliff through an open gap") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    for (int y = 1; y < grid.height; ++y) {
        REQUIRE(setHeight(grid, 3, y, 2));
    }
    const std::vector<PathCell> path = findPath(grid, NavLayer::Ground, 0, 2, 6, 2);
    REQUIRE(path.back().x == 6);
    REQUIRE(path.back().y == 2);
    bool usedGap = false;
    for (const PathCell& cell : path) {
        REQUIRE((cell.x != 3 || cell.y == 0));
        if (cell.x == 3 && cell.y == 0) {
            usedGap = true;
        }
    }
    REQUIRE(usedGap);
}

TEST_CASE("findPath stops on the near side when a cliff blocks every row") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 3));
    for (int y = 0; y < grid.height; ++y) {
        REQUIRE(setHeight(grid, 3, y, 2));
    }
    const std::vector<PathCell> path = findPath(grid, NavLayer::Ground, 0, 1, 6, 1);
    REQUIRE(path.back().x == 2);
    REQUIRE(path.back().y == 1);
}

TEST_CASE("findPath lets ground cross a cliff by the ramp") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 3));
    for (int row = 0; row < grid.height; ++row) {
        REQUIRE(setHeight(grid, 3, row, 2));
        REQUIRE(setPlateau(grid, 4, row, 1));
    }
    REQUIRE(setRamp(grid, 3, 1, 1));
    REQUIRE(setRamp(grid, 5, 1, 0));
    const std::vector<PathCell> path = findPath(grid, NavLayer::Ground, 0, 1, 6, 1);
    REQUIRE(path.back().x == 6);
    REQUIRE(path.back().y == 1);
}
