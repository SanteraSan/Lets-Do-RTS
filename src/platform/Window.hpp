#pragma once

#include <SDL.h>

struct WindowDesc {
    const char* title = "Lets Do RTS";
    int width = 1280;
    int height = 720;
};

inline bool isValidWindowDesc(WindowDesc desc) {
    return desc.title != nullptr && desc.width > 0 && desc.height > 0;
}

struct Window {
    SDL_Window* handle = nullptr;

    Window() = default;
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    ~Window() {
        close();
    }

    bool open(WindowDesc desc);
    void close();
    void pollEvents();

    bool isOpen() const {
        return handle != nullptr;
    }
};
