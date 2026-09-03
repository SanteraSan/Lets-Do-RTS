#pragma once

#include "math/Vector2.hpp"
#include "platform/Camera.hpp"

struct SDL_Renderer;

void drawCircle(SDL_Renderer* renderer, ScreenPoint center, int radiusPixels);
void drawFacingArrow(SDL_Renderer* renderer, ScreenPoint center, Vector2 facing, int radiusPixels);
