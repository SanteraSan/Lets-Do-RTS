#pragma once

#include "nav/Approach.hpp"

#include <cmath>
#include <limits>
#include <queue>
#include <vector>

struct PathCell {
    int x = 0;
    int y = 0;
};

inline float pathHeuristic(int x, int y, int goalX, int goalY) {
    const int dx = x > goalX ? x - goalX : goalX - x;
    const int dy = y > goalY ? y - goalY : goalY - y;
    const int diagonal = dx < dy ? dx : dy;
    const int straight = (dx > dy ? dx : dy) - diagonal;
    return static_cast<float>(straight) + static_cast<float>(diagonal) * 1.41421356f;
}

inline float pathStepCost(int fromX, int fromY, int toX, int toY) {
    if (fromX != toX && fromY != toY) {
        return 1.41421356f;
    }
    return 1.0f;
}

inline std::vector<PathCell> findPath(const Grid& grid, NavLayer layer, int startX, int startY, int goalX, int goalY) {
    std::vector<PathCell> empty;
    if (tileAt(grid, startX, startY) == nullptr || tileAt(grid, goalX, goalY) == nullptr) {
        return empty;
    }
    if (!canOccupyCell(grid, layer, startX, startY)) {
        return empty;
    }
    if (startX == goalX && startY == goalY) {
        return empty;
    }

    const int count = grid.width * grid.height;
    std::vector<float> cost(static_cast<std::size_t>(count), std::numeric_limits<float>::infinity());
    std::vector<int> parent(static_cast<std::size_t>(count), -1);
    std::vector<char> closed(static_cast<std::size_t>(count), 0);
    const auto indexOf = [&](int x, int y) { return y * grid.width + x; };

    struct Node {
        int x = 0;
        int y = 0;
        float rank = 0.0f;
    };
    struct Farther {
        bool operator()(const Node& a, const Node& b) const {
            return a.rank > b.rank;
        }
    };
    std::priority_queue<Node, std::vector<Node>, Farther> open;
    const int startIndex = indexOf(startX, startY);
    cost[static_cast<std::size_t>(startIndex)] = 0.0f;
    open.push(Node{startX, startY, pathHeuristic(startX, startY, goalX, goalY)});

    int bestX = startX;
    int bestY = startY;
    float bestHeuristic = pathHeuristic(startX, startY, goalX, goalY);
    int bestOffset = startY > goalY ? startY - goalY : goalY - startY;
    const int offsets[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    while (!open.empty()) {
        const Node current = open.top();
        open.pop();
        const int currentIndex = indexOf(current.x, current.y);
        if (closed[static_cast<std::size_t>(currentIndex)] != 0) {
            continue;
        }
        closed[static_cast<std::size_t>(currentIndex)] = 1;
        const float currentHeuristic = pathHeuristic(current.x, current.y, goalX, goalY);
        const int currentOffset = current.y > goalY ? current.y - goalY : goalY - current.y;
        if (currentHeuristic < bestHeuristic ||
            (currentHeuristic == bestHeuristic && currentOffset < bestOffset)) {
            bestHeuristic = currentHeuristic;
            bestOffset = currentOffset;
            bestX = current.x;
            bestY = current.y;
        }
        if (current.x == goalX && current.y == goalY) {
            bestX = goalX;
            bestY = goalY;
            break;
        }
        for (const auto& offset : offsets) {
            const int nextX = current.x + offset[0];
            const int nextY = current.y + offset[1];
            if (!canStepCell(grid, layer, current.x, current.y, nextX, nextY)) {
                continue;
            }
            const int nextIndex = indexOf(nextX, nextY);
            if (closed[static_cast<std::size_t>(nextIndex)] != 0) {
                continue;
            }
            const float nextCost =
                cost[static_cast<std::size_t>(currentIndex)] + pathStepCost(current.x, current.y, nextX, nextY);
            if (nextCost >= cost[static_cast<std::size_t>(nextIndex)]) {
                continue;
            }
            cost[static_cast<std::size_t>(nextIndex)] = nextCost;
            parent[static_cast<std::size_t>(nextIndex)] = currentIndex;
            const float rank = nextCost + pathHeuristic(nextX, nextY, goalX, goalY);
            open.push(Node{nextX, nextY, rank});
        }
    }

    if (bestX == startX && bestY == startY) {
        return empty;
    }
    std::vector<PathCell> reversed;
    int x = bestX;
    int y = bestY;
    while (!(x == startX && y == startY)) {
        reversed.push_back(PathCell{x, y});
        const int previous = parent[static_cast<std::size_t>(indexOf(x, y))];
        if (previous < 0) {
            return empty;
        }
        x = previous % grid.width;
        y = previous / grid.width;
    }
    std::vector<PathCell> path;
    for (int i = static_cast<int>(reversed.size()) - 1; i >= 0; --i) {
        path.push_back(reversed[static_cast<std::size_t>(i)]);
    }
    return path;
}

inline std::vector<Vector2> pathTargets(
    const Grid& grid, NavLayer layer, Vector2 from, Vector2 to, float radius) {
    int startX = 0;
    int startY = 0;
    int goalX = 0;
    int goalY = 0;
    if (!worldToCell(grid, from, startX, startY) || !worldToCell(grid, to, goalX, goalY)) {
        return {};
    }
    const std::vector<PathCell> path = findPath(grid, layer, startX, startY, goalX, goalY);
    std::vector<Vector2> points;
    if (path.empty()) {
        return points;
    }
    for (std::size_t i = 0; i + 1 < path.size(); ++i) {
        points.push_back(cellCenter(grid, path[i].x, path[i].y));
    }
    const PathCell& last = path.back();
    points.push_back(approachTarget(grid, layer, cellCenter(grid, last.x, last.y), to, radius));
    return points;
}
