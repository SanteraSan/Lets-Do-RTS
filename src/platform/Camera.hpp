#pragma once

#include "math/Vector2.hpp"

struct Camera {
    Vector2 origin{};
    float pixelsPerUnit = 10.0f;
    float offsetX = 200.0f;
    float offsetY = 360.0f;
};

struct ScreenPoint {
    int x = 0;
    int y = 0;
};

inline ScreenPoint worldToScreen(Vector2 world, Camera camera) {
    return ScreenPoint{
        static_cast<int>(
            camera.offsetX + (world.x - camera.origin.x) * camera.pixelsPerUnit),
        static_cast<int>(
            camera.offsetY + (world.y - camera.origin.y) * camera.pixelsPerUnit)};
}

inline int worldLengthToPixels(float worldLength, Camera camera) {
    if (worldLength <= 0.0f || camera.pixelsPerUnit <= 0.0f) {
        return 0;
    }
    const int pixels = static_cast<int>(worldLength * camera.pixelsPerUnit);
    return pixels < 1 ? 1 : pixels;
}
