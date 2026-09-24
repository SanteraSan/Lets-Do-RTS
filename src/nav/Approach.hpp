#pragma once

#include "nav/Grid.hpp"

#include <cstdlib>

inline bool approachOrder(
    const Grid& grid, NavLayer layer, Vector2 from, Vector2 to, int& stopX, int& stopY) {
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
            return true;
        }
        lastX = x;
        lastY = y;
    }

    stopX = x1;
    stopY = y1;
    return true;
}
