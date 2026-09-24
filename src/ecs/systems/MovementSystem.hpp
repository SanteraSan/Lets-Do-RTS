#pragma once

#include "ecs/World.hpp"
#include "nav/GridMove.hpp"
#include "nav/UnitBlock.hpp"

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

inline void moveSystem(World& world, const Grid& grid, float dt) {
    for (std::size_t i = 0; i < world.entities.size(); ++i) {
        Transform& t = world.transforms[i];
        Mover& m = world.movers[i];
        if (m.ordered) {
            std::vector<PathCell> taken;
            blockingCells(world, i, t.layer, grid, taken);
            steerToOrder(m, t.layer, t.position, grid, taken);
        }
        if (!m.waypoints.empty() && grid.cellSize > 0.0f &&
            distance(t.position, m.target) <= grid.cellSize * 0.05f) {
            m.target = m.waypoints.front();
            m.waypoints.erase(m.waypoints.begin());
        }
        std::vector<PathCell> taken;
        blockingCells(world, i, t.layer, grid, taken);
        const Vector2 previous = t.position;
        moveOnGrid(t.position, m.target, t.layer, grid, m.speed, dt, &taken);
        const Vector2 delta = sub(t.position, previous);
        if (length(delta) > 0.001f) {
            m.facing = normalize(delta);
        }
    }
}
