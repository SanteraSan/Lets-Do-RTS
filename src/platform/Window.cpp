#include "platform/Binding.hpp"
#include "platform/Window.hpp"

bool Window::open(WindowDesc desc) {
    close();
    if (!isValidWindowDesc(desc)) {
        return false;
    }
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return false;
    }
    handle = SDL_CreateWindow(
        desc.title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, desc.width, desc.height, 0);
    if (handle == nullptr) {
        SDL_Quit();
        return false;
    }
    renderer = SDL_CreateRenderer(handle, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        SDL_DestroyWindow(handle);
        handle = nullptr;
        SDL_Quit();
        return false;
    }
    return true;
}

void Window::close() {
    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (handle == nullptr) {
        return;
    }
    SDL_DestroyWindow(handle);
    handle = nullptr;
    SDL_Quit();
}

void Window::pollEvents(InputFrame& input) {
    input.leftClick = false;
    input.rightClick = false;
    input.keyWalk = false;
    input.keyRun = false;
    input.panLeft = false;
    input.panRight = false;
    input.panUp = false;
    input.panDown = false;
    input.middleDown = false;

    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            close();
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            InputAction action = InputAction::Select;
            if (findAction(
                    kDefaultBindings, kDefaultBindingCount, true, event.button.button, action)) {
                raiseAction(input, action);
            }
        } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            InputAction action = InputAction::Select;
            if (findAction(
                    kDefaultBindings, kDefaultBindingCount, false, event.key.keysym.sym, action)) {
                raiseAction(input, action);
            }
        }
    }

    if (handle != nullptr) {
        int mouseX = 0;
        int mouseY = 0;
        const Uint32 buttons = SDL_GetMouseState(&mouseX, &mouseY);
        input.mouse.x = mouseX;
        input.mouse.y = mouseY;
        input.middleDown = (buttons & SDL_BUTTON_MMASK) != 0;

        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        if (keys != nullptr) {
            input.panLeft = keys[SDL_SCANCODE_LEFT] != 0;
            input.panRight = keys[SDL_SCANCODE_RIGHT] != 0;
            input.panUp = keys[SDL_SCANCODE_UP] != 0;
            input.panDown = keys[SDL_SCANCODE_DOWN] != 0;
        }
    }
}

bool Window::beginFrame() {
    if (renderer == nullptr) {
        return false;
    }
    SDL_SetRenderDrawColor(renderer, 20, 24, 32, 255);
    SDL_RenderClear(renderer);
    return true;
}

void Window::endFrame() {
    if (renderer != nullptr) {
        SDL_RenderPresent(renderer);
    }
}
