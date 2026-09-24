#pragma once

#include "platform/Camera.hpp"

#include <vector>

struct GlVertex {
    float x = 0.0f;
    float y = 0.0f;
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
};

struct GlDraw {
    bool ready = false;
    int width = 1;
    int height = 1;
    unsigned vao = 0;
    unsigned vbo = 0;
    unsigned program = 0;
    std::vector<GlVertex> triangles;
    std::vector<GlVertex> lines;

    bool init();
    void shutdown();
    void clear(float red, float green, float blue);
    void triangle(ScreenPoint a, ScreenPoint b, ScreenPoint c, int red, int green, int blue);
    void line(ScreenPoint a, ScreenPoint b, int red, int green, int blue);
    void flush();
};
