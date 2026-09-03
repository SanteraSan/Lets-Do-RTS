#pragma once

#include "math/Vector2.hpp"

inline void moveTowards(Vector2& position, Vector2 target, float speed, float dt) {
    if (speed <= 0.0f || dt <= 0.0f) {
        return;
    }

    const float dist = distance(position, target);
    if (dist == 0.0f) {
        return;
    }

    const float step = speed * dt;
    if (step >= dist) {
        position = target;
        return;
    }

    position = add(position, scale(normalize(sub(target, position)), step));
}
