#pragma once

#include "math/Vector2.hpp"

#include <cmath>

struct RaisedPoint {
    Vector2 position{};
    float height = 0.0f;
};

struct CubeFace {
    RaisedPoint corner[4]{};
    bool front = false;
    bool top = false;
};

struct UnitCube {
    CubeFace faces[3]{};
    int count = 0;
};

inline Vector2 faceCenter(const CubeFace& face) {
    Vector2 sum{};
    for (const RaisedPoint& corner : face.corner) {
        sum = add(sum, corner.position);
    }
    return scale(sum, 0.25f);
}

inline UnitCube visibleUnitCube(Vector2 center, Vector2 facing, float side, float yScale) {
    UnitCube cube;
    if (side <= 0.0f) {
        return cube;
    }
    const Vector2 forward = length(facing) == 0.0f ? Vector2{1.0f, 0.0f} : normalize(facing);
    const Vector2 sideAxis{-forward.y, forward.x};
    const float half = side * 0.5f;
    const Vector2 bottom[4] = {
        add(center, add(scale(forward, half), scale(sideAxis, half))),
        add(center, add(scale(forward, half), scale(sideAxis, -half))),
        add(center, add(scale(forward, -half), scale(sideAxis, -half))),
        add(center, add(scale(forward, -half), scale(sideAxis, half))),
    };

    const float edge = std::sqrt(1.0f + yScale * yScale);
    const float towardCameraZ = edge > 0.0f ? (2.0f * yScale / edge) : 0.0f;

    auto addFace = [&](RaisedPoint a, RaisedPoint b, RaisedPoint c, RaisedPoint d, bool front, bool top) {
        if (cube.count >= 3) {
            return;
        }
        CubeFace& face = cube.faces[cube.count];
        face.corner[0] = a;
        face.corner[1] = b;
        face.corner[2] = c;
        face.corner[3] = d;
        face.front = front;
        face.top = top;
        cube.count += 1;
    };
    auto wall = [&](int first, int second, Vector2 normal, bool front) {
        if (normal.x + normal.y <= 0.0001f) {
            return;
        }
        addFace(
            RaisedPoint{bottom[first], 0.0f},
            RaisedPoint{bottom[second], 0.0f},
            RaisedPoint{bottom[second], side},
            RaisedPoint{bottom[first], side},
            front,
            false);
    };

    wall(0, 1, forward, true);
    wall(1, 2, scale(sideAxis, -1.0f), false);
    wall(2, 3, scale(forward, -1.0f), false);
    wall(3, 0, sideAxis, false);
    if (towardCameraZ > 0.0f) {
        addFace(
            RaisedPoint{bottom[0], side},
            RaisedPoint{bottom[1], side},
            RaisedPoint{bottom[2], side},
            RaisedPoint{bottom[3], side},
            false,
            true);
    }
    return cube;
}
