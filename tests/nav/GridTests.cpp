#include "nav/Grid.hpp"
#include "platform/Camera.hpp"

#include <catch2/catch_approx.hpp>
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

TEST_CASE("ground can occupy a plateau and a ramp") {
    REQUIRE(canOccupy(NavLayer::Ground, Tile{1, false, true, false}));
    REQUIRE(canOccupy(NavLayer::Ground, Tile{1, false, false, true}));
    REQUIRE_FALSE(canOccupy(NavLayer::Underwater, Tile{1, false, true, false}));
}

TEST_CASE("ground steps up only through a ramp") {
    const Tile low{};
    const Tile plateau{1, false, true, false};
    const Tile ramp{1, false, false, true};
    const Tile cliff{2, false, false, false};
    REQUIRE_FALSE(canStep(NavLayer::Ground, low, plateau));
    REQUIRE(canStep(NavLayer::Ground, low, ramp));
    REQUIRE(canStep(NavLayer::Ground, ramp, plateau));
    REQUIRE_FALSE(canStep(NavLayer::Ground, plateau, cliff));
    REQUIRE(canStep(NavLayer::Air, low, cliff));
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

TEST_CASE("underwater can occupy water") {
    REQUIRE(canOccupy(NavLayer::Underwater, Tile{-1}));
}

TEST_CASE("ground cannot occupy water") {
    REQUIRE_FALSE(canOccupy(NavLayer::Ground, Tile{-1}));
}

TEST_CASE("a bridge can be occupied by ground, underwater, and air") {
    const Tile bridge{0, true};
    REQUIRE(canOccupy(NavLayer::Ground, bridge));
    REQUIRE(canOccupy(NavLayer::Underwater, bridge));
    REQUIRE(canOccupy(NavLayer::Air, bridge));
    REQUIRE_FALSE(canOccupy(NavLayer::Surface, bridge));
}

TEST_CASE("clear water lets a surface boat through and keeps ground out") {
    const Tile water{-1, false};
    REQUIRE(canOccupy(NavLayer::Surface, water));
    REQUIRE(canOccupy(NavLayer::Underwater, water));
    REQUIRE_FALSE(canOccupy(NavLayer::Ground, water));
}

TEST_CASE("a cave lets underwater and air through") {
    const Tile cave{2, false, false, false, true, false};
    REQUIRE(canOccupy(NavLayer::Underwater, cave));
    REQUIRE(canOccupy(NavLayer::Air, cave));
    REQUIRE_FALSE(canOccupy(NavLayer::Ground, cave));
    REQUIRE_FALSE(canOccupy(NavLayer::Surface, cave));
}

TEST_CASE("a blocked cell rejects every layer") {
    const Tile blocked{0, false, false, false, false, true};
    REQUIRE_FALSE(canOccupy(NavLayer::Ground, blocked));
    REQUIRE_FALSE(canOccupy(NavLayer::Air, blocked));
    REQUIRE_FALSE(canOccupy(NavLayer::Underwater, blocked));
    REQUIRE_FALSE(canOccupy(NavLayer::Surface, blocked));
}

TEST_CASE("setBridge opens a cell for every layer and rejects an outside cell") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 4));
    REQUIRE(setHeight(grid, 1, 1, -1));
    REQUIRE(setBridge(grid, 1, 1));
    REQUIRE(tileAt(grid, 1, 1)->bridge);
    REQUIRE(canOccupyCell(grid, NavLayer::Ground, 1, 1));
    REQUIRE(canOccupyCell(grid, NavLayer::Underwater, 1, 1));
    REQUIRE_FALSE(setBridge(grid, -1, 0));
}

TEST_CASE("air can occupy water") {
    REQUIRE(canOccupy(NavLayer::Air, Tile{-2}));
}

TEST_CASE("a click on a cell center maps back to that cell") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 8));
    const Camera camera{};
    const Vector2 center = cellCenter(grid, 3, 2);
    const Vector2 world = screenToWorld(worldToScreen(center, camera), camera);
    int x = -1;
    int y = -1;
    REQUIRE(worldToCell(grid, world, x, y));
    REQUIRE(x == 3);
    REQUIRE(y == 2);
}

TEST_CASE("worldToCell maps a point to the cell that contains it") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 4));
    int x = -1;
    int y = -1;
    REQUIRE(worldToCell(grid, Vector2{1.9f, 0.0f}, x, y));
    REQUIRE(x == 1);
    REQUIRE(y == 0);
    REQUIRE(worldToCell(grid, Vector2{2.0f, 3.1f}, x, y));
    REQUIRE(x == 2);
    REQUIRE(y == 3);
}

TEST_CASE("worldToCell rejects a point outside the grid") {
    Grid grid;
    REQUIRE(makeGrid(grid, 2, 2));
    int x = 0;
    int y = 0;
    REQUIRE_FALSE(worldToCell(grid, Vector2{-0.1f, 0.0f}, x, y));
    REQUIRE_FALSE(worldToCell(grid, Vector2{2.0f, 0.0f}, x, y));
    REQUIRE_FALSE(worldToCell(grid, Vector2{0.0f, 2.0f}, x, y));
}

TEST_CASE("worldToCell rejects a non-positive cell size") {
    Grid grid;
    REQUIRE(makeGrid(grid, 2, 2));
    grid.cellSize = 0.0f;
    int x = 0;
    int y = 0;
    REQUIRE_FALSE(worldToCell(grid, Vector2{0.5f, 0.5f}, x, y));
}

TEST_CASE("cellCenter is the middle of the cell") {
    Grid grid;
    grid.cellSize = 4.0f;
    grid.origin = Vector2{-8.0f, 2.0f};
    const Vector2 center = cellCenter(grid, 1, 0);
    REQUIRE(center.x == Catch::Approx(-2.0f));
    REQUIRE(center.y == Catch::Approx(4.0f));
}

TEST_CASE("a cell outside the grid cannot be occupied") {
    Grid grid;
    REQUIRE(makeGrid(grid, 2, 2));
    REQUIRE(setHeight(grid, 0, 1, 2));
    REQUIRE(canOccupyCell(grid, NavLayer::Air, 0, 1));
    REQUIRE_FALSE(canOccupyCell(grid, NavLayer::Ground, 0, 1));
    REQUIRE_FALSE(canOccupyCell(grid, NavLayer::Ground, 3, 0));
}
