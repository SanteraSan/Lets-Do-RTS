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

void drawFacingArrow(SDL_Renderer* renderer, ScreenPoint center, Vector2 facing, int radiusPixels) {
    if (renderer == nullptr || radiusPixels <= 0) {
        return;
    }

    const Vector2 dir = length(facing) == 0.0f ? Vector2{1.0f, 0.0f} : normalize(facing);
    const float shaft = static_cast<float>(radiusPixels) * 1.5f;
    const ScreenPoint tip{
        center.x + static_cast<int>(dir.x * shaft),
        center.y + static_cast<int>(dir.y * shaft)};
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
