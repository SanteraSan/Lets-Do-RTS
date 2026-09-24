#include "nav/Grid.hpp"

#include <catch2/catch_test_macros.hpp>
#include <limits>

TEST_CASE("makeGrid fills every cell with flat ground") {
    Grid grid;
    REQUIRE(makeGrid(grid, 2, 3));
    REQUIRE(grid.width == 2);
    REQUIRE(grid.height == 3);
    REQUIRE(grid.tiles.size() == 6);
    REQUIRE(tileAt(grid, 1, 2) != nullptr);
    REQUIRE(tileAt(grid, 1, 2)->height == 0);
}

TEST_CASE("makeGrid rejects a non-positive size and keeps the previous grid") {
    Grid grid;
    REQUIRE(makeGrid(grid, 2, 2));
    REQUIRE_FALSE(makeGrid(grid, 0, 2));
    REQUIRE_FALSE(makeGrid(grid, 2, -1));
    REQUIRE(grid.width == 2);
    REQUIRE(grid.tiles.size() == 4);
}

TEST_CASE("makeGrid rejects a cell count that does not fit in an int") {
    Grid grid;
    REQUIRE_FALSE(makeGrid(grid, 46341, 46341));
    REQUIRE(grid.tiles.empty());
}

TEST_CASE("cellIndex rejects a coordinate outside the grid") {
    Grid grid;
    REQUIRE(makeGrid(grid, 2, 2));
    REQUIRE(cellIndex(grid, -1, 0) == -1);
    REQUIRE(cellIndex(grid, 2, 0) == -1);
    REQUIRE(cellIndex(grid, 0, 2) == -1);
    REQUIRE(tileAt(grid, 0, -1) == nullptr);
}

TEST_CASE("setHeight stores a cliff and rejects an outside cell") {
    Grid grid;
    REQUIRE(makeGrid(grid, 2, 2));
    REQUIRE(setHeight(grid, 1, 0, 3));
    REQUIRE(tileAt(grid, 1, 0)->height == 3);
    REQUIRE_FALSE(setHeight(grid, -1, 0, 3));
    REQUIRE(tileAt(grid, 0, 0)->height == 0);
}

TEST_CASE("ground can occupy flat ground") {
    REQUIRE(canOccupy(NavLayer::Ground, Tile{0}));
}

TEST_CASE("ground cannot occupy a cliff") {
    REQUIRE_FALSE(canOccupy(NavLayer::Ground, Tile{1}));
}

TEST_CASE("air can occupy a cliff") {
    REQUIRE(canOccupy(NavLayer::Air, Tile{4}));
}

TEST_CASE("air can occupy flat ground") {
    REQUIRE(canOccupy(NavLayer::Air, Tile{0}));
}

TEST_CASE("underwater cannot occupy flat ground or a cliff") {
    REQUIRE_FALSE(canOccupy(NavLayer::Underwater, Tile{0}));
    REQUIRE_FALSE(canOccupy(NavLayer::Underwater, Tile{2}));
}

TEST_CASE("a negative height cannot be occupied yet") {
    REQUIRE_FALSE(canOccupy(NavLayer::Ground, Tile{-1}));
    REQUIRE_FALSE(canOccupy(NavLayer::Air, Tile{-1}));
    REQUIRE_FALSE(canOccupy(NavLayer::Underwater, Tile{-1}));
}

TEST_CASE("a cell outside the grid cannot be occupied") {
    Grid grid;
    REQUIRE(makeGrid(grid, 2, 2));
    REQUIRE(setHeight(grid, 0, 1, 2));
    REQUIRE(canOccupyCell(grid, NavLayer::Air, 0, 1));
    REQUIRE_FALSE(canOccupyCell(grid, NavLayer::Ground, 0, 1));
    REQUIRE_FALSE(canOccupyCell(grid, NavLayer::Ground, 3, 0));
}
