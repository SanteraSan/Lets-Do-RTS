#pragma once

#include "math/Vector2.hpp"
#include "platform/Camera.hpp"
#include "platform/GlDraw.hpp"

void drawCircle(GlDraw& draw, ScreenPoint center, int radiusPixels, float depth, int red, int green, int blue);
void drawFacingArrow(
    GlDraw& draw, ScreenPoint center, ScreenPoint tip, int radiusPixels, float depth, int red, int green, int blue);
void drawQuad(
    GlDraw& draw,
    ScreenPoint a,
    ScreenPoint b,
    ScreenPoint c,
    ScreenPoint d,
    float za,
    float zb,
    float zc,
    float zd,
    int red,
    int green,
    int blue);
void drawDiamond(
    GlDraw& draw,
    ScreenPoint top,
    ScreenPoint right,
    ScreenPoint bottom,
    ScreenPoint left,
    float zTop,
    float zRight,
    float zBottom,
    float zLeft,
    int red,
    int green,
    int blue);
