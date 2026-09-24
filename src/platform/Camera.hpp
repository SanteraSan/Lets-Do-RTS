#pragma once

#include "math/Vector2.hpp"

struct Camera {
    Vector2 origin{};
    float pixelsPerUnit = 10.0f;
    float yScale = 0.5f;
    float offsetX = 640.0f;
    float offsetY = 180.0f;
};

struct ScreenPoint {
    int x = 0;
    int y = 0;
};

inline ScreenPoint worldToScreen(Vector2 world, Camera camera) {
    const float dx = world.x - camera.origin.x;
    const float dy = world.y - camera.origin.y;
    return ScreenPoint{
        static_cast<int>(camera.offsetX + (dx - dy) * camera.pixelsPerUnit),
        static_cast<int>(camera.offsetY + (dx + dy) * camera.pixelsPerUnit * camera.yScale)};
}

inline int worldLengthToPixels(float worldLength, Camera camera) {
    if (worldLength <= 0.0f || camera.pixelsPerUnit <= 0.0f) {
        return 0;
    }
    const int pixels = static_cast<int>(worldLength * camera.pixelsPerUnit);
    return pixels < 1 ? 1 : pixels;
}

inline Vector2 screenToWorld(ScreenPoint screen, Camera camera) {
    if (camera.pixelsPerUnit <= 0.0f || camera.yScale <= 0.0f) {
        return Vector2{};
    }
    const float along = (static_cast<float>(screen.x) - camera.offsetX) / camera.pixelsPerUnit;
    const float across =
        (static_cast<float>(screen.y) - camera.offsetY) / (camera.pixelsPerUnit * camera.yScale);
    return Vector2{
        camera.origin.x + (along + across) * 0.5f,
        camera.origin.y + (across - along) * 0.5f};
}

inline void panCameraPixels(Camera& camera, int screenDx, int screenDy) {
    if (camera.pixelsPerUnit <= 0.0f || camera.yScale <= 0.0f) {
        return;
    }
    if (screenDx == 0 && screenDy == 0) {
        return;
    }
    const ScreenPoint shifted{
        static_cast<int>(camera.offsetX) + screenDx,
        static_cast<int>(camera.offsetY) + screenDy};
    camera.origin = screenToWorld(shifted, camera);
}

inline void panCameraDirections(Camera& camera, bool left, bool right, bool up, bool down, int pixels) {
    if (pixels <= 0) {
        return;
    }
    int screenDx = 0;
    int screenDy = 0;
    if (left) {
        screenDx -= pixels;
    }
    if (right) {
        screenDx += pixels;
    }
    if (up) {
        screenDy -= pixels;
    }
    if (down) {
        screenDy += pixels;
    }
    panCameraPixels(camera, screenDx, screenDy);
}

struct CameraDrag {
    bool active = false;
    ScreenPoint anchor{};
};

inline void dragCamera(Camera& camera, CameraDrag& drag, bool middleDown, ScreenPoint mouse) {
    if (!middleDown) {
        drag.active = false;
        return;
    }
    if (drag.active) {
        panCameraPixels(camera, drag.anchor.x - mouse.x, drag.anchor.y - mouse.y);
    }
    drag.active = true;
    drag.anchor = mouse;
}
