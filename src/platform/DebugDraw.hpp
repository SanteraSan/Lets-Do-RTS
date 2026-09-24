#pragma once

#include "math/Vector2.hpp"
#include "platform/Camera.hpp"

struct SDL_Renderer;

void drawCircle(SDL_Renderer* renderer, ScreenPoint center, int radiusPixels);
void drawFacingArrow(SDL_Renderer* renderer, ScreenPoint center, ScreenPoint tip, int radiusPixels);
void drawDiamond(
    SDL_Renderer* renderer,
    ScreenPoint top,
    ScreenPoint right,
    ScreenPoint bottom,
    ScreenPoint left,
    int red,
    int green,
    int blue);
