#pragma once

#include "math/Vector2.hpp"

enum class NavLayer { Underwater, Ground, Air, Surface };

struct Transform {
    Vector2 position{};
    NavLayer layer = NavLayer::Ground;
};
