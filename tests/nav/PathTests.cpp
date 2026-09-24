#include "nav/Path.hpp"
#include "nav/UnitBlock.hpp"

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

TEST_CASE("findPath crosses the plateau straight to the ramp") {
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 3));
    for (int row = 0; row < grid.height; ++row) {
        REQUIRE(setHeight(grid, 3, row, 2));
        REQUIRE(setPlateau(grid, 4, row, 1));
    }
    REQUIRE(setRamp(grid, 3, 1, 1));
    REQUIRE(setRamp(grid, 5, 1, 0));
    const std::vector<PathCell> path = findPath(grid, NavLayer::Ground, 0, 0, 7, 0);
    bool crossed = false;
    for (const PathCell& cell : path) {
        if (cell.x == 4) {
            REQUIRE(cell.y == 1);
            crossed = true;
        }
    }
    REQUIRE(crossed);
}

TEST_CASE("layers block only another unit on the same plane") {
    REQUIRE(layersBlock(NavLayer::Ground, NavLayer::Ground));
    REQUIRE(layersBlock(NavLayer::Surface, NavLayer::Surface));
    REQUIRE(layersBlock(NavLayer::Underwater, NavLayer::Underwater));
    REQUIRE_FALSE(layersBlock(NavLayer::Surface, NavLayer::Underwater));
    REQUIRE_FALSE(layersBlock(NavLayer::Underwater, NavLayer::Surface));
    REQUIRE_FALSE(layersBlock(NavLayer::Air, NavLayer::Ground));
    REQUIRE_FALSE(layersBlock(NavLayer::Ground, NavLayer::Air));
}

TEST_CASE("findPath walks around another ground unit") {
    Grid grid;
    REQUIRE(makeGrid(grid, 5, 3));
    const std::vector<PathCell> taken{PathCell{2, 1}};
    const std::vector<PathCell> path = findPath(grid, NavLayer::Ground, 0, 1, 4, 1, &taken);
    REQUIRE(path.back().x == 4);
    REQUIRE(path.back().y == 1);
    for (const PathCell& cell : path) {
        REQUIRE((cell.x != 2 || cell.y != 1));
    }
}

TEST_CASE("a boat and a sub do not block each other") {
    Grid grid;
    REQUIRE(makeGrid(grid, 5, 1));
    for (int x = 0; x < grid.width; ++x) {
        REQUIRE(setHeight(grid, x, 0, -1));
    }
    World world;
    const Entity boat = create(world);
    const Entity sub = create(world);
    transform(world, boat)->layer = NavLayer::Surface;
    transform(world, boat)->position = cellCenter(grid, 2, 0);
    transform(world, sub)->layer = NavLayer::Underwater;
    transform(world, sub)->position = cellCenter(grid, 2, 0);

    std::vector<PathCell> taken;
    blockingCells(world, static_cast<std::size_t>(indexOf(world, boat)), NavLayer::Surface, grid, taken);
    REQUIRE(taken.empty());
    const std::vector<PathCell> path = findPath(grid, NavLayer::Surface, 0, 0, 4, 0, &taken);
    REQUIRE(path.back().x == 4);

    blockingCells(world, static_cast<std::size_t>(indexOf(world, sub)), NavLayer::Underwater, grid, taken);
    REQUIRE(taken.empty());
}

TEST_CASE("another boat blocks a surface path") {
    Grid grid;
    REQUIRE(makeGrid(grid, 5, 1));
    for (int x = 0; x < grid.width; ++x) {
        REQUIRE(setHeight(grid, x, 0, -1));
    }
    World world;
    const Entity self = create(world);
    const Entity other = create(world);
    transform(world, self)->layer = NavLayer::Surface;
    transform(world, self)->position = cellCenter(grid, 0, 0);
    transform(world, other)->layer = NavLayer::Surface;
    transform(world, other)->position = cellCenter(grid, 2, 0);

    std::vector<PathCell> taken;
    blockingCells(world, static_cast<std::size_t>(indexOf(world, self)), NavLayer::Surface, grid, taken);
    REQUIRE(taken.size() == 1);
    const std::vector<PathCell> path = findPath(grid, NavLayer::Surface, 0, 0, 4, 0, &taken);
    REQUIRE(path.back().x == 1);
}
