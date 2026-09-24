#pragma once

#include "ecs/World.hpp"
#include "nav/Approach.hpp"
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
    const Vector2 worldPoint = screenToWorld(input.mouse, camera);

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
            selectedMover->target = approachTarget(
                grid, selectedTransform->layer, selectedTransform->position, worldPoint, selectedMover->radius);
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
