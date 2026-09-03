#pragma once

#include "platform/Camera.hpp"

struct InputFrame {
    bool leftClick = false;
    bool rightClick = false;
    bool keyWalk = false;
    bool keyRun = false;
    ScreenPoint mouse{};
};
