#pragma once

#include "ecs/World.hpp"
#include "nav/GridMove.hpp"
#include "nav/UnitBlock.hpp"

constexpr float kBlockedOrderSeconds = 1.0f;

inline void adoptRoute(Mover& mover, NavLayer layer, Vector2 from, const Grid& grid, const std::vector<Vector2>& points) {
    mover.waypoints.clear();
    if (points.empty()) {
        mover.target = approachTarget(grid, layer, from, mover.order, mover.radius);
        return;
    }
    mover.target = points.front();
    mover.waypoints.assign(points.begin() + 1, points.end());
}

inline float routeCost(const Grid& grid, Vector2 from, const std::vector<Vector2>& points) {
    int x0 = 0;
    int y0 = 0;
    if (!worldToCell(grid, from, x0, y0)) {
        return 1.0e9f;
    }
    float cost = 0.0f;
    for (const Vector2& point : points) {
        int x = 0;
        int y = 0;
        if (!worldToCell(grid, point, x, y)) {
            return 1.0e9f;
        }
        cost += pathStepCost(x0, y0, x, y);
        x0 = x;
        y0 = y;
    }
    return cost;
}

inline bool routeOpen(const Grid& grid, NavLayer layer, Vector2 from, const std::vector<Vector2>& points, const std::vector<PathCell>& taken) {
    int x0 = 0;
    int y0 = 0;
    if (!worldToCell(grid, from, x0, y0)) {
        return false;
    }
    for (const Vector2& point : points) {
        int x = 0;
        int y = 0;
        if (!worldToCell(grid, point, x, y)) {
            return false;
        }
        const int dx = x > x0 ? x - x0 : x0 - x;
        const int dy = y > y0 ? y - y0 : y0 - y;
        if ((dx > 1 || dy > 1) ||
            ((x != x0 || y != y0) && (!canStepCell(grid, layer, x0, y0, x, y) || cellTaken(&taken, x, y)))) {
            return false;
        }
        x0 = x;
        y0 = y;
    }
    return true;
}

inline void steerToOrder(
    Mover& mover,
    NavLayer layer,
    Vector2 from,
    const Grid& grid,
    const std::vector<PathCell>& taken,
    bool keepTiedRoute = false) {
    const std::vector<Vector2> points = pathTargets(grid, layer, from, mover.order, mover.radius, &taken);
    std::vector<Vector2> current;
    current.push_back(mover.target);
    current.insert(current.end(), mover.waypoints.begin(), mover.waypoints.end());
    if (keepTiedRoute && routeOpen(grid, layer, from, current, taken)) {
        int goalX = 0;
        int goalY = 0;
        int oldX = 0;
        int oldY = 0;
        int newX = 0;
        int newY = 0;
        const bool goalKnown = worldToCell(grid, mover.order, goalX, goalY);
        const bool oldReaches = goalKnown && worldToCell(grid, current.back(), oldX, oldY) && oldX == goalX && oldY == goalY;
        const bool newReaches = goalKnown && !points.empty() && worldToCell(grid, points.back(), newX, newY) && newX == goalX && newY == goalY;
        const float newCost = points.empty() ? 1.0e9f : routeCost(grid, from, points);
        const float oldCost = routeCost(grid, from, current);
        if (oldReaches && newReaches && !(newCost + 0.5f < oldCost)) {
            return;
        }
    }
    adoptRoute(mover, layer, from, grid, points);
}

inline bool orderStopsShort(
    const Grid& grid,
    NavLayer layer,
    Vector2 from,
    Vector2 order,
    const std::vector<PathCell>& taken,
    bool* noStep = nullptr) {
    if (noStep != nullptr) {
        *noStep = false;
    }
    int startX = 0;
    int startY = 0;
    int goalX = 0;
    int goalY = 0;
    if (!worldToCell(grid, from, startX, startY) || !worldToCell(grid, order, goalX, goalY)) {
        return false;
    }
    if (startX == goalX && startY == goalY) {
        return false;
    }
    const std::vector<PathCell> path = findPath(grid, layer, startX, startY, goalX, goalY, &taken);
    if (path.empty()) {
        if (noStep != nullptr) {
            *noStep = true;
        }
        return true;
    }
    return path.back().x != goalX || path.back().y != goalY;
}

inline Vector2 rampCenterFirst(
    Vector2 position,
    Vector2 target,
    const Grid& grid,
    const std::vector<PathCell>& taken) {
    int fromX = 0;
    int fromY = 0;
    int toX = 0;
    int toY = 0;
    if (!worldToCell(grid, position, fromX, fromY) || !worldToCell(grid, target, toX, toY)) {
        return target;
    }
    const Tile* here = tileAt(grid, fromX, fromY);
    const int stepX = toX - fromX;
    const int stepY = toY - fromY;
    if (here == nullptr || !here->ramp || stepX == 0 || stepY == 0 || stepX > 1 || stepX < -1 || stepY > 1 ||
        stepY < -1) {
        return target;
    }
    if (!cellTaken(&taken, fromX + stepX, fromY) && !cellTaken(&taken, fromX, fromY + stepY)) {
        return target;
    }
    const Vector2 center = cellCenter(grid, fromX, fromY);
    if (distance(center, target) + 0.001f < distance(position, target)) {
        return center;
    }
    return target;
}

inline void moveSystem(World& world, const Grid& grid, float dt) {
    for (std::size_t i = 0; i < world.entities.size(); ++i) {
        Transform& t = world.transforms[i];
        Mover& m = world.movers[i];
        std::vector<PathCell> taken;
        blockingCells(world, i, t.layer, grid, taken);
        if (m.ordered) {
            steerToOrder(m, t.layer, t.position, grid, taken, true);
        }
        if (!m.waypoints.empty() && grid.cellSize > 0.0f &&
            distance(t.position, m.target) <= grid.cellSize * 0.05f) {
            m.target = m.waypoints.front();
            m.waypoints.erase(m.waypoints.begin());
        }
        const Vector2 previous = t.position;
        const Vector2 stepTarget = rampCenterFirst(t.position, m.target, grid, taken);
        moveOnGrid(t.position, stepTarget, t.layer, grid, m.speed, dt, &taken);
        const Vector2 delta = sub(t.position, previous);
        bool noStep = false;
        const bool closed = m.ordered && orderStopsShort(grid, t.layer, t.position, m.order, taken, &noStep);
        if (!closed) {
            m.blockedFor = 0.0f;
        } else if (noStep || m.blockedFor > 0.0f) {
            m.blockedFor += dt;
            if (m.blockedFor >= kBlockedOrderSeconds) {
                m.ordered = false;
                m.blockedFor = 0.0f;
                m.waypoints.clear();
                m.target = t.position;
            }
        }
        if (length(delta) > 0.001f) {
            m.facing = normalize(delta);
        }
    }
}
