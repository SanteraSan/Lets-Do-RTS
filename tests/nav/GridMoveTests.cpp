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

TEST_CASE("moveOnGrid keeps a step that enters the side cell") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    Vector2 position{0.9f, 0.2f};
    moveOnGrid(position, Vector2{3.0f, 0.8f}, NavLayer::Ground, grid, 1.0f, 0.2f);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, position, cellX, cellY));
    REQUIRE(cellX == 1);
    REQUIRE(cellY == 0);
    REQUIRE(position.y == Catch::Approx(0.254944f).margin(0.001f));
}

TEST_CASE("moveOnGrid lets an underwater unit enter water") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    REQUIRE(setHeight(grid, 1, 1, -1));
    REQUIRE(setHeight(grid, 2, 1, -1));
    Vector2 position{1.4f, 1.2f};
    moveOnGrid(position, Vector2{6.2f, 1.2f}, NavLayer::Underwater, grid, 10.0f, 1.0f);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, position, cellX, cellY));
    REQUIRE(cellX == 2);
    REQUIRE(cellY == 1);
}

TEST_CASE("moveOnGrid keeps a ground unit out of water") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    REQUIRE(setHeight(grid, 2, 1, -1));
    Vector2 position{1.4f, 1.2f};
    moveOnGrid(position, Vector2{6.2f, 1.2f}, NavLayer::Ground, grid, 10.0f, 1.0f);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, position, cellX, cellY));
    REQUIRE(cellX == 1);
    REQUIRE(cellY == 1);
}

TEST_CASE("moveOnGrid lets a ground unit enter a bridge") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    REQUIRE(setBridge(grid, 2, 1));
    Vector2 position{1.4f, 1.2f};
    moveOnGrid(position, Vector2{6.2f, 1.2f}, NavLayer::Ground, grid, 10.0f, 1.0f);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, position, cellX, cellY));
    REQUIRE(cellX == 2);
    REQUIRE(cellY == 1);
}

TEST_CASE("moveOnGrid stops before a cell held by another unit") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 1));
    Vector2 position = cellCenter(grid, 0, 0);
    const std::vector<PathCell> taken{PathCell{1, 0}};
    moveOnGrid(position, cellCenter(grid, 3, 0), NavLayer::Ground, grid, 10.0f, 1.0f, &taken);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, position, cellX, cellY));
    REQUIRE(cellX == 0);
    REQUIRE(cellY == 0);
}

TEST_CASE("moveOnGrid enters an open diagonal cell without reversing") {
    Grid grid;
    REQUIRE(makeGrid(grid, 3, 3));
    const std::vector<PathCell> taken{PathCell{1, 1}};
    Vector2 up = cellCenter(grid, 0, 1);
    Vector2 down = cellCenter(grid, 1, 0);
    for (int i = 0; i < 20; ++i) {
        const float beforeX = up.x;
        const float beforeY = up.y;
        moveOnGrid(up, down, NavLayer::Ground, grid, 2.0f, 0.2f, &taken);
        REQUIRE(up.x >= beforeX);
        REQUIRE(up.y <= beforeY);
    }
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, up, cellX, cellY));
    REQUIRE(cellX == 1);
    REQUIRE(cellY == 0);

    for (int i = 0; i < 20; ++i) {
        const float beforeX = down.x;
        const float beforeY = down.y;
        moveOnGrid(down, cellCenter(grid, 0, 1), NavLayer::Ground, grid, 2.0f, 0.2f, &taken);
        REQUIRE(down.x <= beforeX);
        REQUIRE(down.y >= beforeY);
    }
    REQUIRE(worldToCell(grid, down, cellX, cellY));
    REQUIRE(cellX == 0);
    REQUIRE(cellY == 1);
}

TEST_CASE("moveOnGrid approaches a blocked corner without stepping back") {
    Grid grid;
    REQUIRE(makeGrid(grid, 3, 3));
    Vector2 position{0.85f, 1.15f};
    const std::vector<PathCell> taken{PathCell{1, 1}};
    const Vector2 goal = cellCenter(grid, 1, 0);
    for (int i = 0; i < 30; ++i) {
        const float beforeX = position.x;
        const float beforeY = position.y;
        moveOnGrid(position, goal, NavLayer::Ground, grid, 2.0f, 0.15f, &taken);
        REQUIRE(position.x >= beforeX - 0.001f);
        REQUIRE(position.y <= beforeY + 0.001f);
    }
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, position, cellX, cellY));
    REQUIRE(cellX == 1);
    REQUIRE(cellY == 0);
}

TEST_CASE("moveOnGrid ignores a non-positive speed") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 4));
    Vector2 position{0.2f, 0.2f};
    moveOnGrid(position, Vector2{3.0f, 0.2f}, NavLayer::Ground, grid, 0.0f, 1.0f);
    REQUIRE(position.x == Catch::Approx(0.2f));
}
