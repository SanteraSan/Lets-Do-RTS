#pragma once

#include "platform/Input.hpp"

#include <SDL.h>

enum class InputAction { Select, Order, Walk, Run };

struct Binding {
    bool mouse = false;
    int code = 0;
    InputAction action = InputAction::Select;
};

inline constexpr Binding kDefaultBindings[] = {
    {true, SDL_BUTTON_LEFT, InputAction::Select},
    {true, SDL_BUTTON_RIGHT, InputAction::Order},
    {false, SDLK_1, InputAction::Walk},
    {false, SDLK_2, InputAction::Run},
};

inline constexpr int kDefaultBindingCount =
    static_cast<int>(sizeof(kDefaultBindings) / sizeof(kDefaultBindings[0]));

inline bool findAction(
    const Binding* bindings, int count, bool mouse, int code, InputAction& action) {
    if (bindings == nullptr || count <= 0) {
        return false;
    }
    for (int i = 0; i < count; ++i) {
        if (bindings[i].mouse == mouse && bindings[i].code == code) {
            action = bindings[i].action;
            return true;
        }
    }
    return false;
}

inline void raiseAction(InputFrame& input, InputAction action) {
    switch (action) {
    case InputAction::Select:
        input.leftClick = true;
        break;
    case InputAction::Order:
        input.rightClick = true;
        break;
    case InputAction::Walk:
        input.keyWalk = true;
        break;
    case InputAction::Run:
        input.keyRun = true;
        break;
    }
}
