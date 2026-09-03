#pragma once

#include "ecs/Entity.hpp"
#include "ecs/components/Mover.hpp"
#include "ecs/components/Transform.hpp"

#include <cstddef>
#include <vector>

struct World {
    Entity nextId = 1;
    std::vector<Entity> entities;
    std::vector<Transform> transforms;
    std::vector<Mover> movers;
};

inline int indexOf(const World& world, Entity entity) {
    for (std::size_t i = 0; i < world.entities.size(); ++i) {
        if (world.entities[i] == entity) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

inline Entity create(World& world) {
    const Entity id = world.nextId;
    world.nextId += 1;
    world.entities.push_back(id);
    world.transforms.push_back(Transform{});
    world.movers.push_back(Mover{});
    return id;
}

inline bool destroy(World& world, Entity entity) {
    const int index = indexOf(world, entity);
    if (index < 0) {
        return false;
    }
    const std::size_t i = static_cast<std::size_t>(index);
    const std::size_t last = world.entities.size() - 1;
    world.entities[i] = world.entities[last];
    world.transforms[i] = world.transforms[last];
    world.movers[i] = world.movers[last];
    world.entities.pop_back();
    world.transforms.pop_back();
    world.movers.pop_back();
    return true;
}

inline Transform* transform(World& world, Entity entity) {
    const int index = indexOf(world, entity);
    if (index < 0) {
        return nullptr;
    }
    return &world.transforms[static_cast<std::size_t>(index)];
}

inline const Transform* transform(const World& world, Entity entity) {
    const int index = indexOf(world, entity);
    if (index < 0) {
        return nullptr;
    }
    return &world.transforms[static_cast<std::size_t>(index)];
}

inline Mover* mover(World& world, Entity entity) {
    const int index = indexOf(world, entity);
    if (index < 0) {
        return nullptr;
    }
    return &world.movers[static_cast<std::size_t>(index)];
}

inline const Mover* mover(const World& world, Entity entity) {
    const int index = indexOf(world, entity);
    if (index < 0) {
        return nullptr;
    }
    return &world.movers[static_cast<std::size_t>(index)];
}
