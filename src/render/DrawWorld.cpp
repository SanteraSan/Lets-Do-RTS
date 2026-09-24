#include "render/DrawWorld.hpp"

#include "platform/DebugDraw.hpp"
#include "render/Appearance.hpp"

#include <SDL.h>

namespace {

ScreenPoint midpoint(ScreenPoint a, ScreenPoint b) {
    return ScreenPoint{(a.x + b.x) / 2, (a.y + b.y) / 2};
}

} // namespace

void drawWorld(
    SDL_Renderer* renderer, const World& world, const Grid& grid, Camera camera, Selection selection) {
    if (renderer == nullptr) {
        return;
    }

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
            drawDiamond(renderer, top, right, bottom, left, look.fill.red, look.fill.green, look.fill.blue);
            if (look.deck) {
                drawDiamond(
                    renderer,
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
        SDL_SetRenderDrawColor(
            renderer,
            static_cast<Uint8>(color.red),
            static_cast<Uint8>(color.green),
            static_cast<Uint8>(color.blue),
            255);
        const ScreenPoint screen = worldToScreen(world.transforms[i].position, camera);
        const int radiusPixels = worldLengthToPixels(world.movers[i].radius, camera);
        drawCircle(renderer, screen, radiusPixels);
        const Vector2 facing = length(world.movers[i].facing) == 0.0f ? Vector2{1.0f, 0.0f}
                                                                       : normalize(world.movers[i].facing);
        const Vector2 tipWorld =
            add(world.transforms[i].position, scale(facing, world.movers[i].radius * 1.5f));
        drawFacingArrow(renderer, screen, worldToScreen(tipWorld, camera), radiusPixels);

        if (world.entities[i] == selection.entity) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            drawCircle(renderer, screen, radiusPixels + 3);
        }
    }
}
