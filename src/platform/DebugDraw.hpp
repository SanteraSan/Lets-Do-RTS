#pragma once

#include "math/Vector2.hpp"
#include "platform/Camera.hpp"
#include "platform/GlDraw.hpp"

void drawCircle(GlDraw& draw, ScreenPoint center, int radiusPixels, int red, int green, int blue);
void drawFacingArrow(GlDraw& draw, ScreenPoint center, ScreenPoint tip, int radiusPixels, int red, int green, int blue);
void drawQuad(GlDraw& draw, ScreenPoint a, ScreenPoint b, ScreenPoint c, ScreenPoint d, int red, int green, int blue);
void drawDiamond(
    GlDraw& draw,
    ScreenPoint top,
    ScreenPoint right,
    ScreenPoint bottom,
    ScreenPoint left,
    int red,
    int green,
    int blue);
