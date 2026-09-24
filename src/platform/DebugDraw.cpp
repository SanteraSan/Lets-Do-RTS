#include "platform/DebugDraw.hpp"

#include <cmath>

namespace {

constexpr int kCircleSegments = 24;
constexpr float kTau = 6.2831853f;

int lifted(int channel) {
    const int value = channel + 24;
    return value > 255 ? 255 : value;
}

} // namespace

void drawCircle(GlDraw& draw, ScreenPoint center, int radiusPixels, float depth, int red, int green, int blue) {
    if (radiusPixels <= 0) {
        return;
    }
    ScreenPoint previous{center.x + radiusPixels, center.y};
    for (int i = 1; i <= kCircleSegments; ++i) {
        const float angle = kTau * static_cast<float>(i) / static_cast<float>(kCircleSegments);
        const ScreenPoint point{
            center.x + static_cast<int>(std::cos(angle) * static_cast<float>(radiusPixels)),
            center.y + static_cast<int>(std::sin(angle) * static_cast<float>(radiusPixels))};
        draw.line(previous, point, depth, depth, red, green, blue);
        previous = point;
    }
}

void drawFacingArrow(
    GlDraw& draw, ScreenPoint center, ScreenPoint tip, int radiusPixels, float depth, int red, int green, int blue) {
    if (radiusPixels <= 0) {
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
    draw.line(center, tip, depth, depth, red, green, blue);

    const Vector2 left{-dir.y, dir.x};
    const float head = static_cast<float>(radiusPixels) * 0.45f;
    const ScreenPoint a{
        tip.x - static_cast<int>(dir.x * head + left.x * head * 0.6f),
        tip.y - static_cast<int>(dir.y * head + left.y * head * 0.6f)};
    const ScreenPoint b{
        tip.x - static_cast<int>(dir.x * head - left.x * head * 0.6f),
        tip.y - static_cast<int>(dir.y * head - left.y * head * 0.6f)};
    draw.line(tip, a, depth, depth, red, green, blue);
    draw.line(tip, b, depth, depth, red, green, blue);
}

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
    int blue) {
    draw.triangle(a, b, c, za, zb, zc, red, green, blue);
    draw.triangle(a, c, d, za, zc, zd, red, green, blue);
    const int outlineRed = lifted(red);
    const int outlineGreen = lifted(green);
    const int outlineBlue = lifted(blue);
    draw.line(a, b, za, zb, outlineRed, outlineGreen, outlineBlue);
    draw.line(b, c, zb, zc, outlineRed, outlineGreen, outlineBlue);
    draw.line(c, d, zc, zd, outlineRed, outlineGreen, outlineBlue);
    draw.line(d, a, zd, za, outlineRed, outlineGreen, outlineBlue);
}

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
    int blue) {
    draw.triangle(top, right, bottom, zTop, zRight, zBottom, red, green, blue);
    draw.triangle(top, bottom, left, zTop, zBottom, zLeft, red, green, blue);
    const int outlineRed = lifted(red);
    const int outlineGreen = lifted(green);
    const int outlineBlue = lifted(blue);
    draw.line(top, right, zTop, zRight, outlineRed, outlineGreen, outlineBlue);
    draw.line(right, bottom, zRight, zBottom, outlineRed, outlineGreen, outlineBlue);
    draw.line(bottom, left, zBottom, zLeft, outlineRed, outlineGreen, outlineBlue);
    draw.line(left, top, zLeft, zTop, outlineRed, outlineGreen, outlineBlue);
}
