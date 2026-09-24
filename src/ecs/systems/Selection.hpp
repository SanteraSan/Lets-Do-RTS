#pragma once

#include "ecs/World.hpp"
#include "ecs/systems/MovementSystem.hpp"
#include "nav/UnitBlock.hpp"
#include "platform/Input.hpp"
#include "render/StandLift.hpp"
#include "render/UnitCube.hpp"

struct Selection {
    Entity entity = kInvalidEntity;
};

constexpr float kWalkSpeed = 4.0f;
constexpr float kRunSpeed = 14.0f;

inline int triangleCross(ScreenPoint a, ScreenPoint b, ScreenPoint point) {
    return (b.x - a.x) * (point.y - a.y) - (b.y - a.y) * (point.x - a.x);
}

inline bool pointInTriangle(ScreenPoint point, ScreenPoint a, ScreenPoint b, ScreenPoint c) {
    const int ab = triangleCross(a, b, point);
    const int bc = triangleCross(b, c, point);
    const int ca = triangleCross(c, a, point);
    const bool hasNeg = ab < 0 || bc < 0 || ca < 0;
    const bool hasPos = ab > 0 || bc > 0 || ca > 0;
    return !(hasNeg && hasPos);
}

inline bool pointOnFace(ScreenPoint point, ScreenPoint a, ScreenPoint b, ScreenPoint c, ScreenPoint d) {
    return pointInTriangle(point, a, b, c) || pointInTriangle(point, a, c, d);
}

inline bool pickCell(const Grid& grid, ScreenPoint mouse, Camera camera, int& outX, int& outY) {
    bool found = false;
    float bestDepth = 0.0f;
    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            const Tile* tile = tileAt(grid, x, y);
            if (tile == nullptr || grid.cellSize <= 0.0f) {
                continue;
            }
            const float x0 = grid.origin.x + static_cast<float>(x) * grid.cellSize;
            const float y0 = grid.origin.y + static_cast<float>(y) * grid.cellSize;
            const float lift = surfaceLift(tile->height, grid.cellSize);
            const ScreenPoint top = worldToScreen(Vector2{x0, y0}, lift, camera);
            const ScreenPoint right = worldToScreen(Vector2{x0 + grid.cellSize, y0}, lift, camera);
            const ScreenPoint bottom = worldToScreen(Vector2{x0 + grid.cellSize, y0 + grid.cellSize}, lift, camera);
            const ScreenPoint left = worldToScreen(Vector2{x0, y0 + grid.cellSize}, lift, camera);
            if (!pointOnFace(mouse, top, right, bottom, left)) {
                continue;
            }
            const Vector2 center = cellCenter(grid, x, y);
            const float depth = viewDepth(center, lift, camera);
            if (!found || depth < bestDepth) {
                found = true;
                bestDepth = depth;
                outX = x;
                outY = y;
            }
        }
    }
    return found;
}

inline Entity hitUnit(const World& world, ScreenPoint mouse, Camera camera, const Grid& grid) {
    for (int i = static_cast<int>(world.entities.size()) - 1; i >= 0; --i) {
        const std::size_t index = static_cast<std::size_t>(i);
        const float feet = standLift(grid, world.transforms[index].position, world.transforms[index].layer);
        const Vector2 facing = length(world.movers[index].facing) == 0.0f ? Vector2{1.0f, 0.0f}
                                                                           : normalize(world.movers[index].facing);
        const UnitCube cube = visibleUnitCube(
            world.transforms[index].position, facing, world.movers[index].radius * 2.0f, camera.yScale);
        for (int faceIndex = 0; faceIndex < cube.count; ++faceIndex) {
            const CubeFace& face = cube.faces[faceIndex];
            const ScreenPoint a = worldToScreen(face.corner[0].position, face.corner[0].height + feet, camera);
            const ScreenPoint b = worldToScreen(face.corner[1].position, face.corner[1].height + feet, camera);
            const ScreenPoint c = worldToScreen(face.corner[2].position, face.corner[2].height + feet, camera);
            const ScreenPoint d = worldToScreen(face.corner[3].position, face.corner[3].height + feet, camera);
            if (pointOnFace(mouse, a, b, c, d)) {
                return world.entities[index];
            }
        }
    }
    return kInvalidEntity;
}

inline void applyInput(
    World& world, Selection& selection, const InputFrame& input, Camera camera, const Grid& grid) {
    int pickedX = 0;
    int pickedY = 0;
    const bool picked = pickCell(grid, input.mouse, camera, pickedX, pickedY);
    const Vector2 worldPoint = picked ? cellCenter(grid, pickedX, pickedY) : screenToWorld(input.mouse, camera);

    if (input.leftClick) {
        selection.entity = hitUnit(world, input.mouse, camera, grid);
    }

    if (input.rightClick) {
        Mover* selectedMover = mover(world, selection.entity);
        Transform* selectedTransform = transform(world, selection.entity);
        int stopX = 0;
        int stopY = 0;
        if (selectedMover != nullptr && selectedTransform != nullptr &&
            approachOrder(grid, selectedTransform->layer, selectedTransform->position, worldPoint, stopX, stopY)) {
            std::vector<PathCell> taken;
            const int self = indexOf(world, selection.entity);
            if (self >= 0) {
                blockingCells(world, static_cast<std::size_t>(self), selectedTransform->layer, grid, taken);
            }
            selectedMover->order = worldPoint;
            selectedMover->ordered = true;
            selectedMover->blockedFor = 0.0f;
            steerToOrder(
                *selectedMover, selectedTransform->layer, selectedTransform->position, grid, taken);
        }
    }

    Mover* selectedMover = mover(world, selection.entity);
    if (selectedMover == nullptr) {
        return;
    }
    if (input.keyWalk) {
        selectedMover->speed = kWalkSpeed;
    }
    if (input.keyRun) {
        selectedMover->speed = kRunSpeed;
    }
}
