#pragma once

#include "math/Vector2.hpp"
#include "platform/Camera.hpp"

#include <cmath>

constexpr float kInfantryCellsTall = 1.5f;
constexpr float kWalkStride = 2.0f;
constexpr float kWalkBob = 0.6f;

struct SpriteScreen {
    ScreenPoint feet{};
    int pixelWidth = 0;
    int pixelHeight = 0;
    bool flip = false;
    float depth = 0.0f;
};

inline float walkBob(float traveled, bool moving) {
    if (!moving || kWalkStride <= 0.0f) {
        return 0.0f;
    }
    const float cycle = traveled / kWalkStride;
    return std::fabs(std::sin(cycle * 3.14159265f)) * kWalkBob;
}

inline bool infantryFacesRight(Vector2 facing) {
    const Vector2 forward = length(facing) == 0.0f ? Vector2{1.0f, 0.0f} : normalize(facing);
    return forward.x > forward.y;
}

inline SpriteScreen groundSpriteScreen(
    Vector2 position,
    Vector2 facing,
    float stand,
    float cellSize,
    int imageWidth,
    int imageHeight,
    Camera camera,
    float bob = 0.0f) {
    SpriteScreen frame;
    frame.feet = worldToScreen(position, stand + bob, camera);
    const float worldHeight = (cellSize > 0.0f ? cellSize : 1.0f) * kInfantryCellsTall;
    const float edge = std::sqrt(1.0f + camera.yScale * camera.yScale);
    if (camera.pixelsPerUnit > 0.0f && edge > 0.0f) {
        frame.pixelHeight = static_cast<int>(worldHeight * camera.pixelsPerUnit * edge);
    }
    if (frame.pixelHeight < 1) {
        frame.pixelHeight = 1;
    }
    const float aspect = imageWidth > 0 && imageHeight > 0
        ? static_cast<float>(imageWidth) / static_cast<float>(imageHeight)
        : 1.0f;
    frame.pixelWidth = static_cast<int>(static_cast<float>(frame.pixelHeight) * aspect);
    if (frame.pixelWidth < 1) {
        frame.pixelWidth = 1;
    }
    frame.flip = infantryFacesRight(facing);
    frame.depth = viewDepth(position, stand + bob, camera) - 0.15f;
    return frame;
}

inline bool pointOnSprite(ScreenPoint point, SpriteScreen frame) {
    if (frame.pixelWidth <= 0 || frame.pixelHeight <= 0) {
        return false;
    }
    const int left = frame.feet.x - frame.pixelWidth / 2;
    const int right = left + frame.pixelWidth;
    const int bottom = frame.feet.y;
    const int top = bottom - frame.pixelHeight;
    return point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;
}
