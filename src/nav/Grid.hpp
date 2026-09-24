#pragma once

#include "ecs/components/Transform.hpp"

#include <limits>
#include <vector>

struct Tile {
    int height = 0;
    bool bridge = false;
    bool walkable = false;
    bool ramp = false;
    bool cave = false;
    bool blocked = false;
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

inline bool setPlateau(Grid& grid, int x, int y, int height) {
    if (height <= 0) {
        return false;
    }
    const int index = cellIndex(grid, x, y);
    if (index < 0) {
        return false;
    }
    Tile& tile = grid.tiles[static_cast<std::size_t>(index)];
    tile.height = height;
    tile.walkable = true;
    tile.ramp = false;
    tile.bridge = false;
    tile.cave = false;
    tile.blocked = false;
    return true;
}

inline bool setRamp(Grid& grid, int x, int y, int height) {
    if (height < 0) {
        return false;
    }
    const int index = cellIndex(grid, x, y);
    if (index < 0) {
        return false;
    }
    Tile& tile = grid.tiles[static_cast<std::size_t>(index)];
    tile.height = height;
    tile.ramp = true;
    tile.walkable = false;
    tile.bridge = false;
    tile.cave = false;
    tile.blocked = false;
    return true;
}

inline bool setBridge(Grid& grid, int x, int y, int height = 0) {
    if (height < 0) {
        return false;
    }
    const int index = cellIndex(grid, x, y);
    if (index < 0) {
        return false;
    }
    Tile& tile = grid.tiles[static_cast<std::size_t>(index)];
    tile.bridge = true;
    tile.height = height;
    tile.walkable = false;
    tile.ramp = false;
    tile.cave = false;
    tile.blocked = false;
    return true;
}

inline bool setCave(Grid& grid, int x, int y, int height) {
    if (height <= 0) {
        return false;
    }
    const int index = cellIndex(grid, x, y);
    if (index < 0) {
        return false;
    }
    Tile& tile = grid.tiles[static_cast<std::size_t>(index)];
    tile.cave = true;
    tile.height = height;
    tile.bridge = false;
    tile.walkable = false;
    tile.ramp = false;
    tile.blocked = false;
    return true;
}

inline bool setBlocked(Grid& grid, int x, int y) {
    const int index = cellIndex(grid, x, y);
    if (index < 0) {
        return false;
    }
    Tile& tile = grid.tiles[static_cast<std::size_t>(index)];
    tile.blocked = true;
    tile.bridge = false;
    tile.cave = false;
    tile.walkable = false;
    tile.ramp = false;
    return true;
}

inline bool canOccupy(NavLayer layer, Tile tile) {
    if (tile.blocked) {
        return false;
    }
    if (tile.bridge) {
        return layer == NavLayer::Ground || layer == NavLayer::Underwater || layer == NavLayer::Air;
    }
    if (tile.cave) {
        return layer == NavLayer::Underwater || layer == NavLayer::Air;
    }
    if (tile.height < 0) {
        return layer == NavLayer::Underwater || layer == NavLayer::Surface || layer == NavLayer::Air;
    }
    if (tile.ramp || tile.walkable) {
        return layer == NavLayer::Ground || layer == NavLayer::Air;
    }
    if (tile.height > 0) {
        return layer == NavLayer::Air;
    }
    return layer == NavLayer::Ground || layer == NavLayer::Air;
}

inline bool canStep(NavLayer layer, Tile from, Tile to) {
    if (!canOccupy(layer, to)) {
        return false;
    }
    if (layer != NavLayer::Ground) {
        return true;
    }
    const int diff = from.height - to.height;
    const int distance = diff < 0 ? -diff : diff;
    if (distance == 0) {
        return true;
    }
    return distance == 1 && (from.ramp || to.ramp);
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

inline bool canStepCell(const Grid& grid, NavLayer layer, int fromX, int fromY, int toX, int toY) {
    const Tile* from = tileAt(grid, fromX, fromY);
    const Tile* to = tileAt(grid, toX, toY);
    if (from == nullptr || to == nullptr) {
        return false;
    }
    return canStep(layer, *from, *to);
}
