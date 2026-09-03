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

void Window::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            close();
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
