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

inline bool stepCellToward(const Grid& grid, Vector2 from, Vector2 to, int& outX, int& outY) {
    int fromX = 0;
    int fromY = 0;
    int toX = 0;
    int toY = 0;
    if (!worldToCell(grid, from, fromX, fromY) || !worldToCell(grid, to, toX, toY)) {
        return false;
    }
    if (fromX == toX && fromY == toY) {
        return false;
    }
    const int stepX = toX > fromX ? 1 : (toX < fromX ? -1 : 0);
    const int stepY = toY > fromY ? 1 : (toY < fromY ? -1 : 0);
    outX = fromX + stepX;
    outY = fromY + stepY;
    return tileAt(grid, outX, outY) != nullptr;
}

inline bool flushAgainstUnit(const Grid& grid, Vector2 position, Vector2 target, const std::vector<PathCell>& taken) {
    int hereX = 0;
    int hereY = 0;
    if (!worldToCell(grid, position, hereX, hereY)) {
        return false;
    }
    if (cellTaken(&taken, hereX, hereY)) {
        return true;
    }
    int nextX = 0;
    int nextY = 0;
    if (!stepCellToward(grid, position, target, nextX, nextY) || !cellTaken(&taken, nextX, nextY)) {
        return false;
    }
    const float span = distance(cellCenter(grid, hereX, hereY), cellCenter(grid, nextX, nextY));
    if (span <= 0.0f) {
        return false;
    }
    return distance(position, cellCenter(grid, nextX, nextY)) + grid.cellSize * 0.05f < span;
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

inline bool openRampSide(
    const Grid& grid,
    NavLayer layer,
    int fromX,
    int fromY,
    int toX,
    int toY,
    const std::vector<PathCell>& taken,
    int& sideX,
    int& sideY) {
    const int stepX = toX - fromX;
    const int stepY = toY - fromY;
    if (stepX == 0 || stepY == 0 || stepX > 1 || stepX < -1 || stepY > 1 || stepY < -1) {
        return false;
    }
    const int options[2][2] = {{fromX + stepX, fromY}, {fromX, fromY + stepY}};
    for (const auto& option : options) {
        if (canStepCell(grid, layer, fromX, fromY, option[0], option[1]) && !cellTaken(&taken, option[0], option[1])) {
            sideX = option[0];
            sideY = option[1];
            return true;
        }
    }
    return false;
}

inline Vector2 rampCenterFirst(
    Vector2 position,
    Vector2 target,
    NavLayer layer,
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
    if (here == nullptr || !here->ramp || (toX == fromX && toY == fromY)) {
        return target;
    }
    Vector2 exit = target;
    int sideX = 0;
    int sideY = 0;
    if (openRampSide(grid, layer, fromX, fromY, toX, toY, taken, sideX, sideY)) {
        exit = cellCenter(grid, sideX, sideY);
    }
    return exit;
}

inline Vector2 rampCenter(const Grid& grid, Vector2 position) {
    int cellX = 0;
    int cellY = 0;
    if (!worldToCell(grid, position, cellX, cellY)) {
        return position;
    }
    return cellCenter(grid, cellX, cellY);
}

inline void moveSystem(World& world, const Grid& grid, float dt) {
    for (std::size_t i = 0; i < world.entities.size(); ++i) {
        Transform& t = world.transforms[i];
        Mover& m = world.movers[i];
        std::vector<PathCell> taken;
        blockingCells(world, i, t.layer, grid, taken);
        int holdCellX = 0;
        int holdCellY = 0;
        if (worldToCell(grid, t.position, holdCellX, holdCellY) && !cellTaken(&taken, holdCellX, holdCellY)) {
            m.holdX = holdCellX;
            m.holdY = holdCellY;
        }
        if (m.ordered) {
            steerToOrder(m, t.layer, t.position, grid, taken, true);
        }
        if (!m.waypoints.empty() && grid.cellSize > 0.0f &&
            distance(t.position, m.target) <= grid.cellSize * 0.05f) {
            m.target = m.waypoints.front();
            m.waypoints.erase(m.waypoints.begin());
        }
        const Vector2 previous = t.position;
        int cellX = 0;
        int cellY = 0;
        const Tile* here = worldToCell(grid, t.position, cellX, cellY) ? tileAt(grid, cellX, cellY) : nullptr;
        const bool onRamp = here != nullptr && here->ramp;
        if (!onRamp) {
            m.rampHeld = false;
        }
        Vector2 stepTarget = m.target;
        if (onRamp) {
            const Vector2 exit = rampCenterFirst(t.position, m.target, t.layer, grid, taken);
            int exitX = 0;
            int exitY = 0;
            const bool leaving = worldToCell(grid, exit, exitX, exitY) && (exitX != cellX || exitY != cellY);
            const Vector2 center = cellCenter(grid, cellX, cellY);
            if (leaving && !m.rampHeld && distance(t.position, center) > grid.cellSize * 0.05f) {
                stepTarget = center;
            } else if (leaving) {
                m.rampHeld = true;
                stepTarget = exit;
            }
        }
        moveOnGrid(t.position, stepTarget, t.layer, grid, m.speed, dt, &taken);
        const Vector2 delta = sub(t.position, previous);
        bool noStep = false;
        const bool stoppedShort = orderStopsShort(grid, t.layer, t.position, m.order, taken, &noStep);
        const bool againstUnit = flushAgainstUnit(grid, t.position, m.target, taken);
        const bool closed = m.ordered && (stoppedShort || againstUnit);
        if (!closed) {
            m.blockedFor = 0.0f;
        } else if (noStep || againstUnit || m.blockedFor > 0.0f) {
            m.blockedFor += dt;
            if (m.blockedFor >= kBlockedOrderSeconds) {
                m.ordered = false;
                m.blockedFor = 0.0f;
                m.waypoints.clear();
                if (againstUnit && m.holdX >= 0 && m.holdY >= 0) {
                    m.target = cellCenter(grid, m.holdX, m.holdY);
                } else {
                    m.target = t.position;
                }
            }
        }
        const float step = length(delta);
        m.moving = step > 0.001f;
        if (m.moving) {
            m.traveled += step;
            m.facing = normalize(delta);
        }
    }
}
