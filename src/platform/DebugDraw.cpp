#include "platform/DebugDraw.hpp"

#include <SDL.h>
#include <cmath>

namespace {

constexpr int kCircleSegments = 24;
constexpr float kTau = 6.2831853f;

} // namespace

void drawCircle(SDL_Renderer* renderer, ScreenPoint center, int radiusPixels) {
    if (renderer == nullptr || radiusPixels <= 0) {
        return;
    }

    ScreenPoint previous{
        center.x + radiusPixels,
        center.y};
    for (int i = 1; i <= kCircleSegments; ++i) {
        const float angle = kTau * static_cast<float>(i) / static_cast<float>(kCircleSegments);
        const ScreenPoint point{
            center.x + static_cast<int>(std::cos(angle) * static_cast<float>(radiusPixels)),
            center.y + static_cast<int>(std::sin(angle) * static_cast<float>(radiusPixels))};
        SDL_RenderDrawLine(renderer, previous.x, previous.y, point.x, point.y);
        previous = point;
    }
}

void drawFacingArrow(SDL_Renderer* renderer, ScreenPoint center, ScreenPoint tip, int radiusPixels) {
    if (renderer == nullptr || radiusPixels <= 0) {
        return;
    }

    const float dx = static_cast<float>(tip.x - center.x);
    const float dy = static_cast<float>(tip.y - center.y);
    const float shaft = std::sqrt(dx * dx + dy * dy);
    if (shaft <= 0.0f) {
        return;
    }
    const float inv = 1.0f / shaft;
    const Vector2 dir{dx * inv, dy * inv};
    SDL_RenderDrawLine(renderer, center.x, center.y, tip.x, tip.y);

    const Vector2 left{-dir.y, dir.x};
    const float head = static_cast<float>(radiusPixels) * 0.45f;
    const ScreenPoint a{
        tip.x - static_cast<int>(dir.x * head + left.x * head * 0.6f),
        tip.y - static_cast<int>(dir.y * head + left.y * head * 0.6f)};
    const ScreenPoint b{
        tip.x - static_cast<int>(dir.x * head - left.x * head * 0.6f),
        tip.y - static_cast<int>(dir.y * head - left.y * head * 0.6f)};
    SDL_RenderDrawLine(renderer, tip.x, tip.y, a.x, a.y);
    SDL_RenderDrawLine(renderer, tip.x, tip.y, b.x, b.y);
}

void drawDiamond(
    SDL_Renderer* renderer,
    ScreenPoint top,
    ScreenPoint right,
    ScreenPoint bottom,
    ScreenPoint left,
    int red,
    int green,
    int blue) {
    if (renderer == nullptr) {
        return;
    }
    SDL_Vertex verts[4];
    const SDL_Color color{
        static_cast<Uint8>(red),
        static_cast<Uint8>(green),
        static_cast<Uint8>(blue),
        255};
    const ScreenPoint corners[4] = {top, right, bottom, left};
    for (int i = 0; i < 4; ++i) {
        verts[i].position.x = static_cast<float>(corners[i].x);
        verts[i].position.y = static_cast<float>(corners[i].y);
        verts[i].color = color;
        verts[i].tex_coord.x = 0.0f;
        verts[i].tex_coord.y = 0.0f;
    }
    const int indices[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(renderer, nullptr, verts, 4, indices, 6);

    const int outlineRed = red + 24 > 255 ? 255 : red + 24;
    const int outlineGreen = green + 24 > 255 ? 255 : green + 24;
    const int outlineBlue = blue + 24 > 255 ? 255 : blue + 24;
    SDL_SetRenderDrawColor(
        renderer,
        static_cast<Uint8>(outlineRed),
        static_cast<Uint8>(outlineGreen),
        static_cast<Uint8>(outlineBlue),
        255);
    SDL_RenderDrawLine(renderer, top.x, top.y, right.x, right.y);
    SDL_RenderDrawLine(renderer, right.x, right.y, bottom.x, bottom.y);
    SDL_RenderDrawLine(renderer, bottom.x, bottom.y, left.x, left.y);
    SDL_RenderDrawLine(renderer, left.x, left.y, top.x, top.y);
}
