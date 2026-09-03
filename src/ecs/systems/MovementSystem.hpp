#pragma once

#include "ecs/World.hpp"
#include "math/MoveTowards.hpp"

inline void moveSystem(World& world, float dt) {
    for (std::size_t i = 0; i < world.entities.size(); ++i) {
        Transform& t = world.transforms[i];
        Mover& m = world.movers[i];
        const Vector2 previous = t.position;
        moveTowards(t.position, m.target, m.speed, dt);
        const Vector2 delta = sub(t.position, previous);
        if (length(delta) > 0.0f) {
            m.facing = normalize(delta);
        }
    }
}
