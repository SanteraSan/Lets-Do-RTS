#pragma once

#include "ecs/World.hpp"
#include "ecs/systems/Selection.hpp"
#include "nav/Grid.hpp"
#include "platform/Camera.hpp"

struct SDL_Renderer;

void drawWorld(
    SDL_Renderer* renderer, const World& world, const Grid& grid, Camera camera, Selection selection);
