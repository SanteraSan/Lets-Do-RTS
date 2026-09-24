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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    handle = SDL_CreateWindow(
        desc.title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        desc.width,
        desc.height,
        SDL_WINDOW_OPENGL);
    if (handle == nullptr) {
        SDL_Quit();
        return false;
    }
    context = SDL_GL_CreateContext(handle);
    if (context == nullptr || !draw.init()) {
        close();
        return false;
    }
    width = desc.width;
    height = desc.height;
    draw.width = width;
    draw.height = height;
    return true;
}

void Window::close() {
    if (context != nullptr) {
        SDL_GL_MakeCurrent(handle, context);
        draw.shutdown();
        SDL_GL_DeleteContext(context);
        context = nullptr;
    } else {
        draw.shutdown();
    }
    if (handle == nullptr) {
        return;
    }
    SDL_DestroyWindow(handle);
    handle = nullptr;
    width = 0;
    height = 0;
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
    if (context == nullptr || !draw.ready) {
        return false;
    }
    draw.width = width;
    draw.height = height;
    draw.clear(20.0f / 255.0f, 24.0f / 255.0f, 32.0f / 255.0f);
    return true;
}

void Window::endFrame() {
    if (context == nullptr) {
        return;
    }
    draw.flush();
    SDL_GL_SwapWindow(handle);
}
