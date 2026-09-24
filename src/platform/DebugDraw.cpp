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

void drawCircle(GlDraw& draw, ScreenPoint center, int radiusPixels, int red, int green, int blue) {
    if (radiusPixels <= 0) {
        return;
    }
    ScreenPoint previous{center.x + radiusPixels, center.y};
    for (int i = 1; i <= kCircleSegments; ++i) {
        const float angle = kTau * static_cast<float>(i) / static_cast<float>(kCircleSegments);
        const ScreenPoint point{
            center.x + static_cast<int>(std::cos(angle) * static_cast<float>(radiusPixels)),
            center.y + static_cast<int>(std::sin(angle) * static_cast<float>(radiusPixels))};
        draw.line(previous, point, red, green, blue);
        previous = point;
    }
}

void drawFacingArrow(
    GlDraw& draw, ScreenPoint center, ScreenPoint tip, int radiusPixels, int red, int green, int blue) {
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
    draw.line(center, tip, red, green, blue);

    const Vector2 left{-dir.y, dir.x};
    const float head = static_cast<float>(radiusPixels) * 0.45f;
    const ScreenPoint a{
        tip.x - static_cast<int>(dir.x * head + left.x * head * 0.6f),
        tip.y - static_cast<int>(dir.y * head + left.y * head * 0.6f)};
    const ScreenPoint b{
        tip.x - static_cast<int>(dir.x * head - left.x * head * 0.6f),
        tip.y - static_cast<int>(dir.y * head - left.y * head * 0.6f)};
    draw.line(tip, a, red, green, blue);
    draw.line(tip, b, red, green, blue);
}

void drawQuad(GlDraw& draw, ScreenPoint a, ScreenPoint b, ScreenPoint c, ScreenPoint d, int red, int green, int blue) {
    draw.triangle(a, b, c, red, green, blue);
    draw.triangle(a, c, d, red, green, blue);
    const int outlineRed = lifted(red);
    const int outlineGreen = lifted(green);
    const int outlineBlue = lifted(blue);
    draw.line(a, b, outlineRed, outlineGreen, outlineBlue);
    draw.line(b, c, outlineRed, outlineGreen, outlineBlue);
    draw.line(c, d, outlineRed, outlineGreen, outlineBlue);
    draw.line(d, a, outlineRed, outlineGreen, outlineBlue);
}

void drawDiamond(
    GlDraw& draw,
    ScreenPoint top,
    ScreenPoint right,
    ScreenPoint bottom,
    ScreenPoint left,
    int red,
    int green,
    int blue) {
    draw.triangle(top, right, bottom, red, green, blue);
    draw.triangle(top, bottom, left, red, green, blue);
    const int outlineRed = lifted(red);
    const int outlineGreen = lifted(green);
    const int outlineBlue = lifted(blue);
    draw.line(top, right, outlineRed, outlineGreen, outlineBlue);
    draw.line(right, bottom, outlineRed, outlineGreen, outlineBlue);
    draw.line(bottom, left, outlineRed, outlineGreen, outlineBlue);
    draw.line(left, top, outlineRed, outlineGreen, outlineBlue);
}
