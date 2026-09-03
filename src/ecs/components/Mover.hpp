#pragma once

#include "math/Vector2.hpp"

struct Mover {
    Vector2 target{};
    Vector2 facing{1.0f, 0.0f};
    float speed = 0.0f;
    float radius = 2.0f;
};
