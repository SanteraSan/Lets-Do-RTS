#include "render/DrawWorld.hpp"

#include "platform/DebugDraw.hpp"
#include "render/Appearance.hpp"
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
            const float lift = tile->height > 0 ? static_cast<float>(tile->height) * grid.cellSize * 0.5f : 0.0f;
            const ScreenPoint top = worldToScreen(Vector2{x0, y0}, lift, camera);
            const ScreenPoint right = worldToScreen(Vector2{x1, y0}, lift, camera);
            const ScreenPoint bottom = worldToScreen(Vector2{x1, y1}, lift, camera);
            const ScreenPoint left = worldToScreen(Vector2{x0, y1}, lift, camera);
            const TileAppearance look = tileAppearance(*tile);
            if (lift > 0.0f) {
                const ScreenPoint groundTop = worldToScreen(Vector2{x0, y0}, camera);
                const ScreenPoint groundRight = worldToScreen(Vector2{x1, y0}, camera);
                const ScreenPoint groundBottom = worldToScreen(Vector2{x1, y1}, camera);
                const ScreenPoint groundLeft = worldToScreen(Vector2{x0, y1}, camera);
                const int red = shade(look.fill.red, -36);
                const int green = shade(look.fill.green, -36);
                const int blue = shade(look.fill.blue, -36);
                drawQuad(draw, groundTop, groundRight, right, top, red, green, blue);
                drawQuad(draw, groundRight, groundBottom, bottom, right, red, green, blue);
                drawQuad(draw, groundBottom, groundLeft, left, bottom, red, green, blue);
                drawQuad(draw, groundLeft, groundTop, top, left, red, green, blue);
            }
            drawDiamond(draw, top, right, bottom, left, look.fill.red, look.fill.green, look.fill.blue);
            if (look.deck) {
                drawDiamond(
                    draw,
                    midpoint(top, right),
                    midpoint(right, bottom),
                    midpoint(bottom, left),
                    midpoint(left, top),
                    look.deckFill.red,
                    look.deckFill.green,
                    look.deckFill.blue);
            }
        }
    }

    for (std::size_t i = 0; i < world.entities.size(); ++i) {
        const Rgb color = unitColor(world.transforms[i].layer);
        const ScreenPoint screen = worldToScreen(world.transforms[i].position, camera);
        const int radiusPixels = worldLengthToPixels(world.movers[i].radius, camera);
        const Vector2 facing = length(world.movers[i].facing) == 0.0f ? Vector2{1.0f, 0.0f}
                                                                       : normalize(world.movers[i].facing);
        const UnitCube cube =
            visibleUnitCube(world.transforms[i].position, facing, world.movers[i].radius * 2.0f, camera.yScale);
        for (int faceIndex = 0; faceIndex < cube.count; ++faceIndex) {
            const CubeFace& face = cube.faces[faceIndex];
            const int delta = face.front ? 48 : (face.top ? 0 : -48);
            const ScreenPoint a = worldToScreen(face.corner[0].position, face.corner[0].height, camera);
            const ScreenPoint b = worldToScreen(face.corner[1].position, face.corner[1].height, camera);
            const ScreenPoint c = worldToScreen(face.corner[2].position, face.corner[2].height, camera);
            const ScreenPoint d = worldToScreen(face.corner[3].position, face.corner[3].height, camera);
            drawQuad(draw, a, b, c, d, shade(color.red, delta), shade(color.green, delta), shade(color.blue, delta));
            if (face.top) {
                drawQuad(
                    draw,
                    a,
                    b,
                    midpoint(b, c),
                    midpoint(a, d),
                    shade(color.red, 48),
                    shade(color.green, 48),
                    shade(color.blue, 48));
            }
        }
        const Vector2 tipWorld =
            add(world.transforms[i].position, scale(facing, world.movers[i].radius * 1.5f));
        drawFacingArrow(draw, screen, worldToScreen(tipWorld, camera), radiusPixels, color.red, color.green, color.blue);

        if (world.entities[i] == selection.entity) {
            drawCircle(draw, screen, radiusPixels + 3, 255, 255, 255);
        }
    }
}
