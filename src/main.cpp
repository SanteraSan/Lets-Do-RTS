#include "core/FixedTimestep.hpp"
#include "core/Logger.hpp"
#include "ecs/World.hpp"
#include "ecs/systems/MovementSystem.hpp"
#include "ecs/systems/Selection.hpp"
#include "nav/Grid.hpp"
#include "platform/Camera.hpp"
#include "platform/DebugDraw.hpp"
#include "platform/Window.hpp"

#include <iostream>
#include <string>

int main() {
    std::ofstream logFile;
    const bool fileOpen = openLogFile(logFile, "engine.log");

    Logger logger{std::cout, fileOpen ? &logFile : nullptr};
    if (!fileOpen) {
        logger.log(LogLevel::Warn, "could not open engine.log, console only");
    }
    logger.log(LogLevel::Info, "engine start");

    Window window;
    if (!window.open({"Lets Do RTS", 1280, 720})) {
        std::string line = "failed to open window";
        const char* sdlError = SDL_GetError();
        if (sdlError != nullptr && sdlError[0] != '\0') {
            line += ": ";
            line += sdlError;
        }
        logger.log(LogLevel::Error, line);
        return 1;
    }
    logger.log(LogLevel::Info, "window open");

    World world;
    const Entity scout = create(world);
    transform(world, scout)->position = Vector2{0.0f, 0.0f};
    mover(world, scout)->target = Vector2{0.0f, 0.0f};
    mover(world, scout)->speed = 10.0f;
    mover(world, scout)->radius = 2.0f;

    const Entity tanker = create(world);
    transform(world, tanker)->position = Vector2{0.0f, 12.0f};
    mover(world, tanker)->target = Vector2{0.0f, 12.0f};
    mover(world, tanker)->speed = 6.0f;
    mover(world, tanker)->radius = 3.5f;
    transform(world, tanker)->layer = NavLayer::Air;

    Grid grid;
    makeGrid(grid, 40, 24);
    grid.origin = Vector2{-24.0f, -40.0f};
    grid.cellSize = 4.0f;
    for (int y = 0; y < grid.height; ++y) {
        setHeight(grid, 10, y, 2);
    }

    Selection selection;
    InputFrame input;
    FixedTimestep step;
    Uint32 lastTicks = SDL_GetTicks();
    const Camera camera{};

    while (window.isOpen()) {
        window.pollEvents(input);
        if (!window.isOpen()) {
            break;
        }

        applyInput(world, selection, input, camera, grid);

        const Uint32 now = SDL_GetTicks();
        const float rawSeconds = static_cast<float>(now - lastTicks) / 1000.0f;
        lastTicks = now;
        addFrameTime(step, cappedFrameSeconds(rawSeconds));

        while (consumeStep(step)) {
            moveSystem(world, step.delta);
        }

        if (window.beginFrame()) {
            for (std::size_t i = 0; i < world.entities.size(); ++i) {
                if (i == 0) {
                    SDL_SetRenderDrawColor(window.renderer, 220, 220, 80, 255);
                } else {
                    SDL_SetRenderDrawColor(window.renderer, 80, 200, 220, 255);
                }
                const ScreenPoint screen = worldToScreen(world.transforms[i].position, camera);
                const int radiusPixels = worldLengthToPixels(world.movers[i].radius, camera);
                drawCircle(window.renderer, screen, radiusPixels);
                drawFacingArrow(window.renderer, screen, world.movers[i].facing, radiusPixels);

                if (world.entities[i] == selection.entity) {
                    SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
                    drawCircle(window.renderer, screen, radiusPixels + 3);
                }
            }
            window.endFrame();
        }

        SDL_Delay(1);
    }

    logger.log(LogLevel::Info, "engine stop");
    return 0;
}
