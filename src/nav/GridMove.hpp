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
    if (nextX == fromX && nextY == fromY) {
        int towardX = fromX;
        int towardY = fromY;
        if (worldToCell(grid, target, towardX, towardY) && towardX != fromX && towardY != fromY) {
            const bool flankXBlocked =
                cellTaken(taken, towardX, fromY) || !canStepCell(grid, layer, fromX, fromY, towardX, fromY);
            const bool flankYBlocked =
                cellTaken(taken, fromX, towardY) || !canStepCell(grid, layer, fromX, fromY, fromX, towardY);
            if (flankXBlocked || flankYBlocked) {
                const float gapX = target.x > position.x ? target.x - position.x : position.x - target.x;
                const float gapY = target.y > position.y ? target.y - position.y : position.y - target.y;
                Vector2 slide = position;
                if (!flankYBlocked && (gapY >= gapX || flankXBlocked)) {
                    slide.y = next.y;
                } else if (!flankXBlocked) {
                    slide.x = next.x;
                }
                position = slide;
                return;
            }
        }
        position = next;
        return;
    }

    const int spanX = nextX > fromX ? nextX - fromX : fromX - nextX;
    const int spanY = nextY > fromY ? nextY - fromY : fromY - nextY;
    if (spanX <= 1 && spanY <= 1) {
        if (canStepCell(grid, layer, fromX, fromY, nextX, nextY) && !cellTaken(taken, nextX, nextY)) {
            position = next;
            return;
        }
        const float gapX = target.x > position.x ? target.x - position.x : position.x - target.x;
        const float gapY = target.y > position.y ? target.y - position.y : position.y - target.y;
        const int firstAxis = gapY >= gapX ? 1 : 0;
        for (int attempt = 0; attempt < 2; ++attempt) {
            const int axis = attempt == 0 ? firstAxis : 1 - firstAxis;
            Vector2 slide = position;
            if (axis == 0) {
                slide.x = next.x;
            } else {
                slide.y = next.y;
            }
            int slideX = fromX;
            int slideY = fromY;
            if (!worldToCell(grid, slide, slideX, slideY)) {
                clampInsideCell(grid, fromX, fromY, slide);
                position = slide;
                return;
            }
            if ((slideX == fromX && slideY == fromY) ||
                (canStepCell(grid, layer, fromX, fromY, slideX, slideY) && !cellTaken(taken, slideX, slideY))) {
                position = slide;
                return;
            }
        }
        clampInsideCell(grid, fromX, fromY, next);
        position = next;
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
