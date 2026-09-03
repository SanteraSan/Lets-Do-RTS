#pragma once

#include <cmath>

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;
};

inline Vector2 add(Vector2 a, Vector2 b) {
    return Vector2{a.x + b.x, a.y + b.y};
}

inline Vector2 sub(Vector2 a, Vector2 b) {
    return Vector2{a.x - b.x, a.y - b.y};
}

inline Vector2 scale(Vector2 v, float s) {
    return Vector2{v.x * s, v.y * s};
}

inline float length(Vector2 v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

inline Vector2 normalize(Vector2 v) {
    const float len = length(v);
    if (len == 0.0f) {
        return Vector2{};
    }
    return scale(v, 1.0f / len);
}

inline bool almostEqual(Vector2 a, Vector2 b, float epsilon = 0.0001f) {
    return std::fabs(a.x - b.x) <= epsilon && std::fabs(a.y - b.y) <= epsilon;
}
