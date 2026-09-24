#pragma once

#include "nav/Grid.hpp"
#include "nav/Path.hpp"
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
    Vector2& position,
    Vector2 target,
    NavLayer layer,
    const Grid& grid,
    float speed,
    float dt,
    const std::vector<PathCell>* taken = nullptr) {
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
    auto openCell = [&](int cellX, int cellY) {
        return canStepCell(grid, layer, fromX, fromY, cellX, cellY) && !cellTaken(taken, cellX, cellY);
    };
    int towardX = fromX;
    int towardY = fromY;
    const bool towardKnown = worldToCell(grid, target, towardX, towardY);
    const int towardSpanX = towardX > fromX ? towardX - fromX : fromX - towardX;
    const int towardSpanY = towardY > fromY ? towardY - fromY : fromY - towardY;
    const bool towardOpen = towardKnown && (towardSpanX <= 1 && towardSpanY <= 1) &&
        !(towardX == fromX && towardY == fromY) && openCell(towardX, towardY);

    if (nextX == fromX && nextY == fromY) {
        position = next;
        return;
    }

    const int spanX = nextX > fromX ? nextX - fromX : fromX - nextX;
    const int spanY = nextY > fromY ? nextY - fromY : fromY - nextY;
    if (spanX <= 1 && spanY <= 1) {
        if (openCell(nextX, nextY)) {
            position = next;
            return;
        }
        Vector2 alongX = position;
        alongX.x = next.x;
        Vector2 alongY = position;
        alongY.y = next.y;
        auto staysOpen = [&](Vector2 step) {
            int stepX = 0;
            int stepY = 0;
            if (!worldToCell(grid, step, stepX, stepY)) {
                return false;
            }
            return (stepX == fromX && stepY == fromY) || openCell(stepX, stepY);
        };
        const bool xOpen = staysOpen(alongX);
        const bool yOpen = staysOpen(alongY);
        if (xOpen && yOpen) {
            const float gapX = target.x > position.x ? target.x - position.x : position.x - target.x;
            const float gapY = target.y > position.y ? target.y - position.y : position.y - target.y;
            position = gapY > gapX ? alongY : alongX;
            return;
        }
        if (xOpen) {
            position = alongX;
            return;
        }
        if (yOpen) {
            position = alongY;
            return;
        }
        return;
    }

    const int stepX = nextX > fromX ? 1 : (nextX < fromX ? -1 : 0);
    const int stepY = nextY > fromY ? 1 : (nextY < fromY ? -1 : 0);
    if (canStepCell(grid, layer, fromX, fromY, fromX + stepX, fromY + stepY) &&
        !cellTaken(taken, fromX + stepX, fromY + stepY)) {
        clampInsideCell(grid, fromX + stepX, fromY + stepY, next);
    } else {
        clampInsideCell(grid, fromX, fromY, next);
    }
    position = next;
}
