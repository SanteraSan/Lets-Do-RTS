#pragma once

#include "ecs/components/Transform.hpp"
#include "nav/Grid.hpp"

inline float surfaceLift(int height, float cellSize) {
    if (height <= 0 || cellSize <= 0.0f) {
        return 0.0f;
    }
    return static_cast<float>(height) * cellSize * 0.5f;
}

inline float standLift(const Grid& grid, Vector2 position, NavLayer layer) {
    if (layer != NavLayer::Ground && layer != NavLayer::Air) {
        return 0.0f;
    }
    int cellX = 0;
    int cellY = 0;
    if (!worldToCell(grid, position, cellX, cellY)) {
        return 0.0f;
    }
    const Tile* tile = tileAt(grid, cellX, cellY);
    if (tile == nullptr) {
        return 0.0f;
    }
    const float surface = surfaceLift(tile->height, grid.cellSize);
    if (layer == NavLayer::Air) {
        return surface + grid.cellSize;
    }
    return surface;
}
