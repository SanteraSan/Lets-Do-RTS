#pragma once

#include "math/Vector2.hpp"

#include <vector>

struct Mover {
    Vector2 target{};
    Vector2 facing{1.0f, 0.0f};
    Vector2 order{};
    float speed = 0.0f;
    float radius = 2.0f;
    float blockedFor = 0.0f;
    float traveled = 0.0f;
    int holdX = -1;
    int holdY = -1;
    bool moving = false;
    bool ordered = false;
    bool rampHeld = false;
    std::vector<Vector2> waypoints{};
};
