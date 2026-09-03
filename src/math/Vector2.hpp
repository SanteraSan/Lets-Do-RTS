#pragma once

#include <cmath>

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;
};

inline Vector2 add(Vector2 a, Vector2 b) {
    return Vector2{a.x + b.x, a.y + b.y};
}

inline float length(Vector2 v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}
