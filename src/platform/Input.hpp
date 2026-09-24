#pragma once

#include "platform/Camera.hpp"

struct InputFrame {
    bool leftClick = false;
    bool rightClick = false;
    bool keyWalk = false;
    bool keyRun = false;
    bool panLeft = false;
    bool panRight = false;
    bool panUp = false;
    bool panDown = false;
    bool middleDown = false;
    ScreenPoint mouse{};
};
