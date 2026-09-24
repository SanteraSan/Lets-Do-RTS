#pragma once

#include "ecs/World.hpp"
#include "ecs/systems/Selection.hpp"
#include "nav/Grid.hpp"
#include "platform/Camera.hpp"

#include "platform/GlDraw.hpp"

struct SDL_Renderer;

void drawWorld(GlDraw& draw, const World& world, const Grid& grid, Camera camera, Selection selection);
