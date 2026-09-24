#include "render/DrawWorld.hpp"

#include "platform/DebugDraw.hpp"
#include "render/Appearance.hpp"
#include "render/StandLift.hpp"
#include "render/UnitCube.hpp"

namespace {

ScreenPoint midpoint(ScreenPoint a, ScreenPoint b) {
    return ScreenPoint{(a.x + b.x) / 2, (a.y + b.y) / 2};
}

int shade(int channel, int delta) {
    const int value = channel + delta;
    if (value < 0) {
        return 0;
    }
    return value > 255 ? 255 : value;
}

} // namespace

void drawWorld(GlDraw& draw, const World& world, const Grid& grid, Camera camera, Selection selection) {
    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            const Tile* tile = tileAt(grid, x, y);
            if (tile == nullptr) {
                continue;
            }
            const float x0 = grid.origin.x + static_cast<float>(x) * grid.cellSize;
            const float y0 = grid.origin.y + static_cast<float>(y) * grid.cellSize;
            const float x1 = x0 + grid.cellSize;
            const float y1 = y0 + grid.cellSize;
            const float lift = surfaceLift(tile->height, grid.cellSize);
            const Vector2 topWorld{x0, y0};
            const Vector2 rightWorld{x1, y0};
            const Vector2 bottomWorld{x1, y1};
            const Vector2 leftWorld{x0, y1};
            const float zTop = viewDepth(topWorld, lift, camera);
            const float zRight = viewDepth(rightWorld, lift, camera);
            const float zBottom = viewDepth(bottomWorld, lift, camera);
            const float zLeft = viewDepth(leftWorld, lift, camera);
            const ScreenPoint top = worldToScreen(topWorld, lift, camera);
            const ScreenPoint right = worldToScreen(rightWorld, lift, camera);
            const ScreenPoint bottom = worldToScreen(bottomWorld, lift, camera);
            const ScreenPoint left = worldToScreen(leftWorld, lift, camera);
            const TileAppearance look = tileAppearance(*tile);
            if (lift > 0.0f) {
                const float gTop = viewDepth(topWorld, 0.0f, camera);
                const float gRight = viewDepth(rightWorld, 0.0f, camera);
                const float gBottom = viewDepth(bottomWorld, 0.0f, camera);
                const float gLeft = viewDepth(leftWorld, 0.0f, camera);
                const ScreenPoint groundTop = worldToScreen(topWorld, camera);
                const ScreenPoint groundRight = worldToScreen(rightWorld, camera);
                const ScreenPoint groundBottom = worldToScreen(bottomWorld, camera);
                const ScreenPoint groundLeft = worldToScreen(leftWorld, camera);
                const int red = shade(look.fill.red, -36);
                const int green = shade(look.fill.green, -36);
                const int blue = shade(look.fill.blue, -36);
                drawQuad(draw, groundTop, groundRight, right, top, gTop, gRight, zRight, zTop, red, green, blue);
                drawQuad(draw, groundRight, groundBottom, bottom, right, gRight, gBottom, zBottom, zRight, red, green, blue);
                drawQuad(draw, groundBottom, groundLeft, left, bottom, gBottom, gLeft, zLeft, zBottom, red, green, blue);
                drawQuad(draw, groundLeft, groundTop, top, left, gLeft, gTop, zTop, zLeft, red, green, blue);
            }
            drawDiamond(draw, top, right, bottom, left, zTop, zRight, zBottom, zLeft, look.fill.red, look.fill.green, look.fill.blue);
            if (look.deck) {
                const float zNorth = (zTop + zRight) * 0.5f;
                const float zEast = (zRight + zBottom) * 0.5f;
                const float zSouth = (zBottom + zLeft) * 0.5f;
                const float zWest = (zLeft + zTop) * 0.5f;
                drawDiamond(
                    draw,
                    midpoint(top, right),
                    midpoint(right, bottom),
                    midpoint(bottom, left),
                    midpoint(left, top),
                    zNorth,
                    zEast,
                    zSouth,
                    zWest,
                    look.deckFill.red,
                    look.deckFill.green,
                    look.deckFill.blue);
            }
        }
    }

    for (std::size_t i = 0; i < world.entities.size(); ++i) {
        const Rgb color = unitColor(world.transforms[i].layer);
        const float feet = standLift(grid, world.transforms[i].position, world.transforms[i].layer);
        const ScreenPoint screen = worldToScreen(world.transforms[i].position, feet, camera);
        const int radiusPixels = worldLengthToPixels(world.movers[i].radius, camera);
        const Vector2 facing = length(world.movers[i].facing) == 0.0f ? Vector2{1.0f, 0.0f}
                                                                       : normalize(world.movers[i].facing);
        const UnitCube cube =
            visibleUnitCube(world.transforms[i].position, facing, world.movers[i].radius * 2.0f, camera.yScale);
        for (int faceIndex = 0; faceIndex < cube.count; ++faceIndex) {
            const CubeFace& face = cube.faces[faceIndex];
            const int delta = face.front ? 48 : (face.top ? 0 : -48);
            const float ha = face.corner[0].height + feet;
            const float hb = face.corner[1].height + feet;
            const float hc = face.corner[2].height + feet;
            const float hd = face.corner[3].height + feet;
            const float za = viewDepth(face.corner[0].position, ha, camera);
            const float zb = viewDepth(face.corner[1].position, hb, camera);
            const float zc = viewDepth(face.corner[2].position, hc, camera);
            const float zd = viewDepth(face.corner[3].position, hd, camera);
            const ScreenPoint a = worldToScreen(face.corner[0].position, ha, camera);
            const ScreenPoint b = worldToScreen(face.corner[1].position, hb, camera);
            const ScreenPoint c = worldToScreen(face.corner[2].position, hc, camera);
            const ScreenPoint d = worldToScreen(face.corner[3].position, hd, camera);
            drawQuad(draw, a, b, c, d, za, zb, zc, zd, shade(color.red, delta), shade(color.green, delta), shade(color.blue, delta));
            if (face.top) {
                constexpr float kStripeLift = 0.2f;
                const float zaStripe = viewDepth(face.corner[0].position, ha + kStripeLift, camera);
                const float zbStripe = viewDepth(face.corner[1].position, hb + kStripeLift, camera);
                const float zcStripe = viewDepth(face.corner[2].position, hc + kStripeLift, camera);
                const float zdStripe = viewDepth(face.corner[3].position, hd + kStripeLift, camera);
                const ScreenPoint stripeA = worldToScreen(face.corner[0].position, ha + kStripeLift, camera);
                const ScreenPoint stripeB = worldToScreen(face.corner[1].position, hb + kStripeLift, camera);
                const ScreenPoint stripeC = worldToScreen(face.corner[2].position, hc + kStripeLift, camera);
                const ScreenPoint stripeD = worldToScreen(face.corner[3].position, hd + kStripeLift, camera);
                drawQuad(
                    draw,
                    stripeA,
                    stripeB,
                    midpoint(stripeB, stripeC),
                    midpoint(stripeA, stripeD),
                    zaStripe,
                    zbStripe,
                    (zbStripe + zcStripe) * 0.5f,
                    (zaStripe + zdStripe) * 0.5f,
                    shade(color.red, 48),
                    shade(color.green, 48),
                    shade(color.blue, 48));
            }
        }
        const float roof = feet + world.movers[i].radius * 2.0f;
        const float mark = viewDepth(world.transforms[i].position, roof, camera);
        const Vector2 tipWorld =
            add(world.transforms[i].position, scale(facing, world.movers[i].radius * 1.5f));
        drawFacingArrow(
            draw,
            screen,
            worldToScreen(tipWorld, feet, camera),
            radiusPixels,
            mark,
            color.red,
            color.green,
            color.blue);

        if (world.entities[i] == selection.entity) {
            drawCircle(draw, screen, radiusPixels + 3, mark, 255, 255, 255);
        }
    }
}
