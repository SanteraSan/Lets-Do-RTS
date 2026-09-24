#include "render/DrawWorld.hpp"

#include "platform/DebugDraw.hpp"
#include "render/Appearance.hpp"

namespace {

ScreenPoint midpoint(ScreenPoint a, ScreenPoint b) {
    return ScreenPoint{(a.x + b.x) / 2, (a.y + b.y) / 2};
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
            const ScreenPoint top = worldToScreen(Vector2{x0, y0}, camera);
            const ScreenPoint right = worldToScreen(Vector2{x1, y0}, camera);
            const ScreenPoint bottom = worldToScreen(Vector2{x1, y1}, camera);
            const ScreenPoint left = worldToScreen(Vector2{x0, y1}, camera);
            const TileAppearance look = tileAppearance(*tile);
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
        const int spriteHeight = radiusPixels * 3;
        const int spriteWidth = radiusPixels * 2;
        draw.sprite(screen, spriteWidth, spriteHeight, color.red, color.green, color.blue);
        const Vector2 facing = length(world.movers[i].facing) == 0.0f ? Vector2{1.0f, 0.0f}
                                                                       : normalize(world.movers[i].facing);
        const Vector2 tipWorld =
            add(world.transforms[i].position, scale(facing, world.movers[i].radius * 1.5f));
        drawFacingArrow(draw, screen, worldToScreen(tipWorld, camera), radiusPixels, color.red, color.green, color.blue);

        if (world.entities[i] == selection.entity) {
            drawCircle(draw, screen, radiusPixels + 3, 255, 255, 255);
        }
    }
}
