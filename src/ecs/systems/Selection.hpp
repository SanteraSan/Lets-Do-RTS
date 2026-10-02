#pragma once

#include "ecs/EventBus.hpp"
#include "ecs/World.hpp"
#include "ecs/systems/MovementSystem.hpp"
#include "nav/UnitBlock.hpp"
#include "platform/Input.hpp"
#include "render/GroundSprite.hpp"
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

inline Entity hitUnit(
    const World& world, ScreenPoint mouse, Camera camera, const Grid& grid, int pictureWidth = 0, int pictureHeight = 0) {
    for (int i = static_cast<int>(world.entities.size()) - 1; i >= 0; --i) {
        const std::size_t index = static_cast<std::size_t>(i);
        const float feet = standLift(grid, world.transforms[index].position, world.transforms[index].layer);
        const Vector2 facing = length(world.movers[index].facing) == 0.0f ? Vector2{1.0f, 0.0f}
                                                                           : normalize(world.movers[index].facing);
        if (world.transforms[index].layer == NavLayer::Ground && pictureWidth > 0 && pictureHeight > 0) {
            const float bob = walkBob(world.movers[index].traveled, world.movers[index].moving);
            const SpriteScreen frame = groundSpriteScreen(
                world.transforms[index].position,
                facing,
                feet,
                grid.cellSize,
                pictureWidth,
                pictureHeight,
                camera,
                bob);
            if (pointOnSprite(mouse, frame)) {
                return world.entities[index];
            }
            continue;
        }
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

struct OrderEvent {
    Entity unit = kInvalidEntity;
    Vector2 point{};
};

struct SelectEvent {
    Entity unit = kInvalidEntity;
};

struct SpeedEvent {
    Entity unit = kInvalidEntity;
    float speed = 0.0f;
};

inline void applySpeed(World& world, SpeedEvent event) {
    Mover* selectedMover = mover(world, event.unit);
    if (selectedMover == nullptr) {
        return;
    }
    selectedMover->speed = event.speed;
}

inline void connectSpeed(EventBus<SpeedEvent>& bus, World& world) {
    subscribe<SpeedEvent>(bus, [&world](const SpeedEvent& event) { applySpeed(world, event); });
}

inline void applySelection(Selection& selection, SelectEvent event) {
    selection.entity = event.unit;
}

inline void connectSelection(EventBus<SelectEvent>& bus, Selection& selection) {
    subscribe<SelectEvent>(bus, [&selection](const SelectEvent& event) { applySelection(selection, event); });
}

inline void issueOrder(World& world, const Grid& grid, OrderEvent order) {
    Mover* selectedMover = mover(world, order.unit);
    Transform* selectedTransform = transform(world, order.unit);
    int stopX = 0;
    int stopY = 0;
    if (selectedMover == nullptr || selectedTransform == nullptr ||
        !approachOrder(grid, selectedTransform->layer, selectedTransform->position, order.point, stopX, stopY)) {
        return;
    }
    std::vector<PathCell> taken;
    const int self = indexOf(world, order.unit);
    if (self >= 0) {
        blockingCells(world, static_cast<std::size_t>(self), selectedTransform->layer, grid, taken);
    }
    selectedMover->order = order.point;
    selectedMover->ordered = true;
    selectedMover->blockedFor = 0.0f;
    selectedMover->rampHeld = false;
    steerToOrder(*selectedMover, selectedTransform->layer, selectedTransform->position, grid, taken);
}

inline void connectOrders(EventBus<OrderEvent>& bus, World& world, const Grid& grid) {
    subscribe<OrderEvent>(bus, [&world, &grid](const OrderEvent& order) { issueOrder(world, grid, order); });
}

inline void applyInput(
    World& world,
    Selection& selection,
    const InputFrame& input,
    Camera camera,
    const Grid& grid,
    EventBus<OrderEvent>& orders,
    EventBus<SelectEvent>& selections,
    EventBus<SpeedEvent>& speeds,
    int pictureWidth = 0,
    int pictureHeight = 0) {
    int pickedX = 0;
    int pickedY = 0;
    const bool picked = pickCell(grid, input.mouse, camera, pickedX, pickedY);
    const Vector2 worldPoint = picked ? cellCenter(grid, pickedX, pickedY) : screenToWorld(input.mouse, camera);

    if (input.leftClick) {
        publish(selections, SelectEvent{hitUnit(world, input.mouse, camera, grid, pictureWidth, pictureHeight)});
    }

    if (input.rightClick && mover(world, selection.entity) != nullptr && transform(world, selection.entity) != nullptr) {
        int stopX = 0;
        int stopY = 0;
        if (approachOrder(
                grid,
                transform(world, selection.entity)->layer,
                transform(world, selection.entity)->position,
                worldPoint,
                stopX,
                stopY)) {
            publish(orders, OrderEvent{selection.entity, worldPoint});
        }
    }

    if (mover(world, selection.entity) == nullptr) {
        return;
    }
    if (input.keyWalk) {
        publish(speeds, SpeedEvent{selection.entity, kWalkSpeed});
    }
    if (input.keyRun) {
        publish(speeds, SpeedEvent{selection.entity, kRunSpeed});
    }
}

inline void applyInput(
    World& world,
    Selection& selection,
    const InputFrame& input,
    Camera camera,
    const Grid& grid,
    int pictureWidth = 0,
    int pictureHeight = 0) {
    EventBus<OrderEvent> orders;
    EventBus<SelectEvent> selections;
    EventBus<SpeedEvent> speeds;
    connectOrders(orders, world, grid);
    connectSelection(selections, selection);
    connectSpeed(speeds, world);
    applyInput(world, selection, input, camera, grid, orders, selections, speeds, pictureWidth, pictureHeight);
}
