#pragma once

#include "ecs/World.hpp"
#include "nav/GridMove.hpp"
#include "nav/UnitBlock.hpp"

constexpr float kBlockedOrderSeconds = 1.0f;

inline void steerToOrder(Mover& mover, NavLayer layer, Vector2 from, const Grid& grid, const std::vector<PathCell>& taken) {
    const std::vector<Vector2> points = pathTargets(grid, layer, from, mover.order, mover.radius, &taken);
    mover.waypoints.clear();
    if (points.empty()) {
        mover.target = approachTarget(grid, layer, from, mover.order, mover.radius);
        return;
    }
    mover.target = points.front();
    mover.waypoints.assign(points.begin() + 1, points.end());
}

inline bool orderStopsShort(
    const Grid& grid, NavLayer layer, Vector2 from, Vector2 order, const std::vector<PathCell>& taken) {
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
        return true;
    }
    return path.back().x != goalX || path.back().y != goalY;
}

inline void moveSystem(World& world, const Grid& grid, float dt) {
    for (std::size_t i = 0; i < world.entities.size(); ++i) {
        Transform& t = world.transforms[i];
        Mover& m = world.movers[i];
        std::vector<PathCell> taken;
        blockingCells(world, i, t.layer, grid, taken);
        if (m.ordered) {
            steerToOrder(m, t.layer, t.position, grid, taken);
        }
        if (!m.waypoints.empty() && grid.cellSize > 0.0f &&
            distance(t.position, m.target) <= grid.cellSize * 0.05f) {
            m.target = m.waypoints.front();
            m.waypoints.erase(m.waypoints.begin());
        }
        const Vector2 previous = t.position;
        moveOnGrid(t.position, m.target, t.layer, grid, m.speed, dt, &taken);
        const Vector2 delta = sub(t.position, previous);
        const bool pressed = m.ordered && length(delta) <= 0.001f &&
            orderStopsShort(grid, t.layer, t.position, m.order, taken);
        if (pressed) {
            m.blockedFor += dt;
            if (m.blockedFor >= kBlockedOrderSeconds) {
                m.ordered = false;
                m.blockedFor = 0.0f;
                m.waypoints.clear();
                m.target = t.position;
            }
        } else {
            m.blockedFor = 0.0f;
        }
        if (length(delta) > 0.001f) {
            m.facing = normalize(delta);
        }
    }
}
