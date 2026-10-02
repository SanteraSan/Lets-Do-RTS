#include "core/FixedTimestep.hpp"
#include "core/Logger.hpp"
#include "ecs/World.hpp"
#include "ecs/systems/MovementSystem.hpp"
#include "ecs/systems/Selection.hpp"
#include "nav/Grid.hpp"
#include "platform/Camera.hpp"
#include "platform/Window.hpp"
#include "render/DrawWorld.hpp"
#include "render/Png.hpp"

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

    SpriteImage infantry;
    if (!loadPngFile(RTS_INFANTRY_PATH, infantry)) {
        logger.log(LogLevel::Warn, "could not load infantry sprite");
        infantry = makeUnitSprite();
    } else {
        trimSprite(infantry);
    }
    window.draw.setSprite(infantry);

    World world;
    const Entity scout = create(world);
    transform(world, scout)->position = Vector2{0.0f, 0.0f};
    mover(world, scout)->target = Vector2{0.0f, 0.0f};
    mover(world, scout)->speed = 10.0f;
    mover(world, scout)->radius = 2.0f;

    Grid grid;
    makeGrid(grid, 40, 24);
    grid.origin = Vector2{-24.0f, -40.0f};
    grid.cellSize = 4.0f;
    for (int y = 0; y < grid.height; ++y) {
        setHeight(grid, 10, y, 2);
    }
    for (int x = 11; x <= 13; ++x) {
        for (int y = 0; y < grid.height; ++y) {
            if (y != 18) {
                setPlateau(grid, x, y, 1);
            }
        }
    }
    setRamp(grid, 10, 10, 1);
    setRamp(grid, 14, 10, 0);
    for (int y = 16; y <= 21; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            if (x == 10 || (x >= 11 && x <= 13 && y != 18)) {
                continue;
            }
            setHeight(grid, x, y, -1);
        }
    }

    for (int x = 3; x <= 7; ++x) {
        setBridge(grid, x, 18);
    }
    setCave(grid, 10, 18, 2);
    setBridge(grid, 11, 18, 1);
    setBridge(grid, 12, 18, 1);

    const Entity scoutB = create(world);
    transform(world, scoutB)->position = cellCenter(grid, 3, 9);
    mover(world, scoutB)->target = transform(world, scoutB)->position;
    mover(world, scoutB)->speed = 10.0f;
    mover(world, scoutB)->radius = 2.0f;

    const Entity scoutC = create(world);
    transform(world, scoutC)->position = cellCenter(grid, 6, 6);
    mover(world, scoutC)->target = transform(world, scoutC)->position;
    mover(world, scoutC)->speed = 10.0f;
    mover(world, scoutC)->radius = 2.0f;

    const Entity scoutD = create(world);
    transform(world, scoutD)->position = cellCenter(grid, 1, 12);
    mover(world, scoutD)->target = transform(world, scoutD)->position;
    mover(world, scoutD)->speed = 10.0f;
    mover(world, scoutD)->radius = 2.0f;

    const Entity tanker = create(world);
    transform(world, tanker)->position = Vector2{0.0f, 12.0f};
    mover(world, tanker)->target = Vector2{0.0f, 12.0f};
    mover(world, tanker)->speed = 6.0f;
    mover(world, tanker)->radius = 3.5f;
    transform(world, tanker)->layer = NavLayer::Air;

    const Entity tankerB = create(world);
    transform(world, tankerB)->position = cellCenter(grid, 6, 14);
    mover(world, tankerB)->target = transform(world, tankerB)->position;
    mover(world, tankerB)->speed = 6.0f;
    mover(world, tankerB)->radius = 3.5f;
    transform(world, tankerB)->layer = NavLayer::Air;

    const Entity boat = create(world);
    transform(world, boat)->layer = NavLayer::Surface;
    transform(world, boat)->position = cellCenter(grid, 1, 18);
    mover(world, boat)->target = transform(world, boat)->position;
    mover(world, boat)->speed = 8.0f;
    mover(world, boat)->radius = 1.8f;

    const Entity boatB = create(world);
    transform(world, boatB)->layer = NavLayer::Surface;
    transform(world, boatB)->position = cellCenter(grid, 16, 17);
    mover(world, boatB)->target = transform(world, boatB)->position;
    mover(world, boatB)->speed = 8.0f;
    mover(world, boatB)->radius = 1.8f;

    const Entity sub = create(world);
    transform(world, sub)->layer = NavLayer::Underwater;
    transform(world, sub)->position = cellCenter(grid, 4, 18);
    mover(world, sub)->target = transform(world, sub)->position;
    mover(world, sub)->speed = 8.0f;
    mover(world, sub)->radius = 1.6f;

    const Entity subB = create(world);
    transform(world, subB)->layer = NavLayer::Underwater;
    transform(world, subB)->position = cellCenter(grid, 20, 19);
    mover(world, subB)->target = transform(world, subB)->position;
    mover(world, subB)->speed = 8.0f;
    mover(world, subB)->radius = 1.6f;

    Selection selection;
    EventBus<OrderEvent> orders;
    EventBus<SelectEvent> selections;
    EventBus<SpeedEvent> speeds;
    connectOrders(orders, world, grid);
    connectSelection(selections, selection);
    connectSpeed(speeds, world);
    InputFrame input;
    FixedTimestep step;
    Uint32 lastTicks = SDL_GetTicks();
    Camera camera{};
    CameraDrag cameraDrag;

    while (window.isOpen()) {
        window.pollEvents(input);
        if (!window.isOpen()) {
            break;
        }

        constexpr int kPanPixels = 12;
        panCameraDirections(camera, input.panLeft, input.panRight, input.panUp, input.panDown, kPanPixels);
        dragCamera(camera, cameraDrag, input.middleDown, input.mouse);
        applyInput(
            world, selection, input, camera, grid, orders, selections, speeds, infantry.width, infantry.height);

        const Uint32 now = SDL_GetTicks();
        const float rawSeconds = static_cast<float>(now - lastTicks) / 1000.0f;
        lastTicks = now;
        addFrameTime(step, cappedFrameSeconds(rawSeconds));

        while (consumeStep(step)) {
            moveSystem(world, grid, step.delta);
        }

        if (window.beginFrame()) {
            drawWorld(window.draw, world, grid, camera, selection, infantry.width, infantry.height);
            window.endFrame();
        }

        SDL_Delay(1);
    }

    logger.log(LogLevel::Info, "engine stop");
    return 0;
}
