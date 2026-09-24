#pragma once

#include "ecs/components/Transform.hpp"
#include "nav/Grid.hpp"

inline float surfaceLift(int height, float cellSize) {
    if (height <= 0 || cellSize <= 0.0f) {
        return 0.0f;
    }
    return static_cast<float>(height) * cellSize * 0.5f;
}

inline float cellSurfaceLift(const Grid& grid, Vector2 position) {
    int cellX = 0;
    int cellY = 0;
    if (!worldToCell(grid, position, cellX, cellY)) {
        return 0.0f;
    }
    const Tile* tile = tileAt(grid, cellX, cellY);
    if (tile == nullptr) {
        return 0.0f;
    }
    return surfaceLift(tile->height, grid.cellSize);
}

inline float standLift(const Grid& grid, Vector2 position, NavLayer layer) {
    const float surface = cellSurfaceLift(grid, position);
    if (layer == NavLayer::Air) {
        return surface + grid.cellSize;
    }
    if (layer == NavLayer::Underwater) {
        return surface - grid.cellSize;
    }
    if (layer != NavLayer::Ground) {
        return 0.0f;
    }
    return surface;
}
