#pragma once

#include "nav/Grid.hpp"

#include <cstdlib>

inline bool approachOrder(
    const Grid& grid,
    NavLayer layer,
    Vector2 from,
    Vector2 to,
    int& stopX,
    int& stopY,
    int* blockedX = nullptr,
    int* blockedY = nullptr) {
    int x0 = 0;
    int y0 = 0;
    int x1 = 0;
    int y1 = 0;
    if (!worldToCell(grid, from, x0, y0) || !worldToCell(grid, to, x1, y1)) {
        return false;
    }
    if (!canOccupyCell(grid, layer, x0, y0)) {
        return false;
    }

    int x = x0;
    int y = y0;
    int lastX = x0;
    int lastY = y0;
    const int dx = std::abs(x1 - x0);
    const int dy = std::abs(y1 - y0);
    const int stepX = x0 < x1 ? 1 : -1;
    const int stepY = y0 < y1 ? 1 : -1;
    int error = dx - dy;

    while (x != x1 || y != y1) {
        const int error2 = error * 2;
        if (error2 > -dy) {
            error -= dy;
            x += stepX;
        }
        if (error2 < dx) {
            error += dx;
            y += stepY;
        }
        if (!canStepCell(grid, layer, lastX, lastY, x, y)) {
            stopX = lastX;
            stopY = lastY;
            if (blockedX != nullptr) {
                *blockedX = x;
            }
            if (blockedY != nullptr) {
                *blockedY = y;
            }
            return true;
        }
        lastX = x;
        lastY = y;
    }

    stopX = x1;
    stopY = y1;
    if (blockedX != nullptr) {
        *blockedX = -1;
    }
    if (blockedY != nullptr) {
        *blockedY = -1;
    }
    return true;
}

inline Vector2 approachTarget(
    const Grid& grid, NavLayer layer, Vector2 from, Vector2 to, float radius) {
    int stopX = 0;
    int stopY = 0;
    int blockedX = -1;
    int blockedY = -1;
    if (!approachOrder(grid, layer, from, to, stopX, stopY, &blockedX, &blockedY)) {
        return to;
    }
    const Vector2 center = cellCenter(grid, stopX, stopY);
    if (blockedX < 0 || blockedY < 0 || radius < 0.0f) {
        return center;
    }
    const Vector2 blockedCenter = cellCenter(grid, blockedX, blockedY);
    const Vector2 toward = sub(blockedCenter, center);
    const float span = length(toward);
    if (span <= 0.0f || grid.cellSize <= 0.0f) {
        return center;
    }
    const float boundary = span * 0.5f;
    const float keep = grid.cellSize * 0.001f;
    float travel = boundary - radius;
    if (travel < 0.0f) {
        travel = 0.0f;
    }
    const float limit = boundary - keep;
    if (travel > limit) {
        travel = limit;
    }
    return add(center, scale(toward, travel / span));
}
