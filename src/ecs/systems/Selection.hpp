#pragma once

#include "ecs/World.hpp"
#include "nav/Approach.hpp"
#include "platform/Input.hpp"

struct Selection {
    Entity entity = kInvalidEntity;
};

constexpr float kWalkSpeed = 4.0f;
constexpr float kRunSpeed = 14.0f;

inline Entity hitUnit(const World& world, Vector2 worldPoint) {
    for (int i = static_cast<int>(world.entities.size()) - 1; i >= 0; --i) {
        const std::size_t index = static_cast<std::size_t>(i);
        if (distance(worldPoint, world.transforms[index].position) <= world.movers[index].radius) {
            return world.entities[index];
        }
    }
    return kInvalidEntity;
}

inline void applyInput(
    World& world, Selection& selection, const InputFrame& input, Camera camera, const Grid& grid) {
    const Vector2 worldPoint = screenToWorld(input.mouse, camera);

    if (input.leftClick) {
        selection.entity = hitUnit(world, worldPoint);
    }

    if (input.rightClick) {
        Mover* selectedMover = mover(world, selection.entity);
        Transform* selectedTransform = transform(world, selection.entity);
        int stopX = 0;
        int stopY = 0;
        if (selectedMover != nullptr && selectedTransform != nullptr &&
            approachOrder(grid, selectedTransform->layer, selectedTransform->position, worldPoint, stopX, stopY)) {
            selectedMover->target = cellCenter(grid, stopX, stopY);
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
