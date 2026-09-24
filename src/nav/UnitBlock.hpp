#pragma once

#include "ecs/World.hpp"
#include "nav/Path.hpp"

inline bool layersBlock(NavLayer mover, NavLayer other) {
    if (mover == NavLayer::Air || other == NavLayer::Air) {
        return false;
    }
    return mover == other;
}

inline void blockingCells(
    const World& world, std::size_t self, NavLayer layer, const Grid& grid, std::vector<PathCell>& out) {
    out.clear();
    for (std::size_t i = 0; i < world.entities.size(); ++i) {
        if (i == self || !layersBlock(layer, world.transforms[i].layer)) {
            continue;
        }
        int x = 0;
        int y = 0;
        if (worldToCell(grid, world.transforms[i].position, x, y)) {
            out.push_back(PathCell{x, y});
        }
    }
}
