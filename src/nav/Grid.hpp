#pragma once

#include "ecs/components/Transform.hpp"

#include <limits>
#include <vector>

struct Tile {
    int height = 0;
};

struct Grid {
    int width = 0;
    int height = 0;
    float cellSize = 1.0f;
    Vector2 origin{};
    std::vector<Tile> tiles;
};

inline bool makeGrid(Grid& grid, int width, int height) {
    if (width <= 0 || height <= 0) {
        return false;
    }
    const long long count = static_cast<long long>(width) * static_cast<long long>(height);
    if (count > static_cast<long long>(std::numeric_limits<int>::max())) {
        return false;
    }
    grid.width = width;
    grid.height = height;
    grid.tiles.assign(static_cast<std::size_t>(count), Tile{});
    return true;
}

inline int cellIndex(const Grid& grid, int x, int y) {
    if (x < 0 || y < 0 || grid.width <= 0 || grid.height <= 0 || x >= grid.width || y >= grid.height) {
        return -1;
    }
    const long long index = static_cast<long long>(y) * static_cast<long long>(grid.width) + static_cast<long long>(x);
    if (index < 0 || index >= static_cast<long long>(grid.tiles.size())) {
        return -1;
    }
    return static_cast<int>(index);
}

inline const Tile* tileAt(const Grid& grid, int x, int y) {
    const int index = cellIndex(grid, x, y);
    if (index < 0) {
        return nullptr;
    }
    return &grid.tiles[static_cast<std::size_t>(index)];
}

inline bool setHeight(Grid& grid, int x, int y, int height) {
    const int index = cellIndex(grid, x, y);
    if (index < 0) {
        return false;
    }
    grid.tiles[static_cast<std::size_t>(index)].height = height;
    return true;
}

inline bool canOccupy(NavLayer layer, Tile tile) {
    if (tile.height < 0) {
        return false;
    }
    if (tile.height > 0) {
        return layer == NavLayer::Air;
    }
    return layer == NavLayer::Ground || layer == NavLayer::Air;
}

inline bool worldToCell(const Grid& grid, Vector2 world, int& cellX, int& cellY) {
    if (grid.cellSize <= 0.0f) {
        return false;
    }
    const float localX = (world.x - grid.origin.x) / grid.cellSize;
    const float localY = (world.y - grid.origin.y) / grid.cellSize;
    if (localX < 0.0f || localY < 0.0f) {
        return false;
    }
    if (localX >= static_cast<float>(grid.width) || localY >= static_cast<float>(grid.height)) {
        return false;
    }
    cellX = static_cast<int>(localX);
    cellY = static_cast<int>(localY);
    return cellIndex(grid, cellX, cellY) >= 0;
}

inline Vector2 cellCenter(const Grid& grid, int x, int y) {
    return Vector2{
        grid.origin.x + (static_cast<float>(x) + 0.5f) * grid.cellSize,
        grid.origin.y + (static_cast<float>(y) + 0.5f) * grid.cellSize};
}

inline bool canOccupyCell(const Grid& grid, NavLayer layer, int x, int y) {
    const Tile* tile = tileAt(grid, x, y);
    if (tile == nullptr) {
        return false;
    }
    return canOccupy(layer, *tile);
}
