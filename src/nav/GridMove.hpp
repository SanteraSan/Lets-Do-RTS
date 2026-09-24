#pragma once

#include "nav/Grid.hpp"
#include "math/MoveTowards.hpp"

inline void clampInsideCell(const Grid& grid, int cellX, int cellY, Vector2& position) {
    const float minX = grid.origin.x + static_cast<float>(cellX) * grid.cellSize;
    const float minY = grid.origin.y + static_cast<float>(cellY) * grid.cellSize;
    const float inset = grid.cellSize * 0.001f;
    const float maxX = minX + grid.cellSize - inset;
    const float maxY = minY + grid.cellSize - inset;
    if (position.x < minX) {
        position.x = minX;
    } else if (position.x > maxX) {
        position.x = maxX;
    }
    if (position.y < minY) {
        position.y = minY;
    } else if (position.y > maxY) {
        position.y = maxY;
    }
}

inline void moveOnGrid(
    Vector2& position, Vector2 target, NavLayer layer, const Grid& grid, float speed, float dt) {
    if (speed <= 0.0f || dt <= 0.0f || grid.cellSize <= 0.0f) {
        return;
    }
    int fromX = 0;
    int fromY = 0;
    if (!worldToCell(grid, position, fromX, fromY) || !canOccupyCell(grid, layer, fromX, fromY)) {
        return;
    }

    Vector2 next = position;
    moveTowards(next, target, speed, dt);

    int nextX = 0;
    int nextY = 0;
    if (!worldToCell(grid, next, nextX, nextY)) {
        clampInsideCell(grid, fromX, fromY, next);
        position = next;
        return;
    }
    if (nextX == fromX && nextY == fromY) {
        position = next;
        return;
    }

    const int spanX = nextX > fromX ? nextX - fromX : fromX - nextX;
    const int spanY = nextY > fromY ? nextY - fromY : fromY - nextY;
    if (spanX <= 1 && spanY <= 1) {
        if (canOccupyCell(grid, layer, nextX, nextY)) {
            position = next;
            return;
        }
        clampInsideCell(grid, fromX, fromY, next);
        position = next;
        return;
    }

    const int stepX = nextX > fromX ? 1 : (nextX < fromX ? -1 : 0);
    const int stepY = nextY > fromY ? 1 : (nextY < fromY ? -1 : 0);
    if (canOccupyCell(grid, layer, fromX + stepX, fromY + stepY)) {
        clampInsideCell(grid, fromX + stepX, fromY + stepY, next);
    } else {
        clampInsideCell(grid, fromX, fromY, next);
    }
    position = next;
}
