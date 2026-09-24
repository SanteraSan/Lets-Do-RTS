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

    int towardX = fromX;
    int towardY = fromY;
    if (target.x > position.x) {
        towardX = fromX + 1;
    } else if (target.x < position.x) {
        towardX = fromX - 1;
    }
    if (target.y > position.y) {
        towardY = fromY + 1;
    } else if (target.y < position.y) {
        towardY = fromY - 1;
    }
    const bool neighborOpen =
        (towardX != fromX || towardY != fromY) && canOccupyCell(grid, layer, towardX, towardY);

    int nextX = 0;
    int nextY = 0;
    if (worldToCell(grid, next, nextX, nextY) && nextX == fromX && nextY == fromY) {
        position = next;
        return;
    }
    if (worldToCell(grid, next, nextX, nextY) && neighborOpen && nextX == towardX && nextY == towardY) {
        position = next;
        return;
    }
    if (neighborOpen) {
        clampInsideCell(grid, towardX, towardY, next);
    } else {
        clampInsideCell(grid, fromX, fromY, next);
    }
    position = next;
}
