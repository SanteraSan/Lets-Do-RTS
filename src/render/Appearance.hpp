#pragma once

#include "ecs/components/Transform.hpp"
#include "nav/Grid.hpp"

struct Rgb {
    int red = 0;
    int green = 0;
    int blue = 0;
};

struct TileAppearance {
    Rgb fill{};
    bool deck = false;
    Rgb deckFill{};
};

inline TileAppearance tileAppearance(Tile tile) {
    if (tile.bridge) {
        return TileAppearance{Rgb{20, 70, 120}, true, Rgb{24, 36, 44}};
    }
    if (tile.height < 0) {
        return TileAppearance{Rgb{20, 70, 120}, false, {}};
    }
    if (tile.ramp) {
        return TileAppearance{Rgb{180, 150, 60}, false, {}};
    }
    if (tile.walkable && tile.height > 0) {
        return TileAppearance{Rgb{78, 110, 78}, false, {}};
    }
    if (tile.height > 0) {
        return TileAppearance{Rgb{120, 96, 48}, false, {}};
    }
    return TileAppearance{Rgb{24, 36, 44}, false, {}};
}

inline Rgb unitColor(NavLayer layer) {
    if (layer == NavLayer::Air) {
        return Rgb{80, 200, 220};
    }
    if (layer == NavLayer::Underwater) {
        return Rgb{80, 140, 255};
    }
    return Rgb{220, 220, 80};
}
