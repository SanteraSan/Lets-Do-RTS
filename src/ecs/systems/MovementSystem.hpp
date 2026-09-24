#pragma once

#include "ecs/World.hpp"
#include "nav/GridMove.hpp"

inline void moveSystem(World& world, const Grid& grid, float dt) {
    for (std::size_t i = 0; i < world.entities.size(); ++i) {
        Transform& t = world.transforms[i];
        Mover& m = world.movers[i];
        const Vector2 previous = t.position;
        moveOnGrid(t.position, m.target, t.layer, grid, m.speed, dt);
        const Vector2 delta = sub(t.position, previous);
        if (length(delta) > 0.0f) {
            m.facing = normalize(delta);
        }
    }
}
