#include "ecs/World.hpp"
#include "ecs/systems/MovementSystem.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("create makes two different alive entities") {
    World world;
    const Entity a = create(world);
    const Entity b = create(world);
    REQUIRE(a != kInvalidEntity);
    REQUIRE(b != kInvalidEntity);
    REQUIRE(a != b);
    REQUIRE(transform(world, a) != nullptr);
    REQUIRE(transform(world, b) != nullptr);
}

TEST_CASE("transform of an unknown entity is null") {
    World world;
    REQUIRE(transform(world, 99) == nullptr);
}

TEST_CASE("new transform defaults to Ground") {
    World world;
    const Entity id = create(world);
    REQUIRE(transform(world, id)->layer == NavLayer::Ground);
}

TEST_CASE("destroy removes the entity and its components") {
    World world;
    const Entity id = create(world);
    REQUIRE(destroy(world, id));
    REQUIRE_FALSE(destroy(world, id));
    REQUIRE(transform(world, id) == nullptr);
    REQUIRE(mover(world, id) == nullptr);
}

TEST_CASE("moveSystem advances every entity toward its target") {
    World world;
    const Entity a = create(world);
    const Entity b = create(world);
    transform(world, a)->position = Vector2{0.2f, 0.2f};
    mover(world, a)->target = Vector2{10.0f, 0.2f};
    mover(world, a)->speed = 0.4f;
    transform(world, b)->position = Vector2{0.2f, 2.2f};
    mover(world, b)->target = Vector2{10.0f, 2.2f};
    mover(world, b)->speed = 0.4f;

    Grid grid;
    REQUIRE(makeGrid(grid, 16, 8));
    moveSystem(world, grid, 1.0f);

    REQUIRE(transform(world, a)->position.x == Catch::Approx(0.6f));
    REQUIRE(transform(world, b)->position.x == Catch::Approx(0.6f));
    REQUIRE(mover(world, a)->facing.x == Catch::Approx(1.0f));
}

TEST_CASE("moveSystem records the distance a walking unit covers") {
    World world;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{0.2f, 0.2f};
    mover(world, id)->target = Vector2{10.0f, 0.2f};
    mover(world, id)->speed = 0.4f;
    Grid grid;
    REQUIRE(makeGrid(grid, 16, 8));

    moveSystem(world, grid, 1.0f);

    REQUIRE(mover(world, id)->moving);
    REQUIRE(mover(world, id)->traveled == Catch::Approx(0.4f));
}

TEST_CASE("moveSystem marks a unit still when it does not move") {
    World world;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{0.2f, 0.2f};
    mover(world, id)->target = Vector2{0.2f, 0.2f};
    mover(world, id)->speed = 10.0f;
    Grid grid;
    REQUIRE(makeGrid(grid, 16, 8));

    moveSystem(world, grid, 1.0f);

    REQUIRE(mover(world, id)->moving == false);
    REQUIRE(mover(world, id)->traveled == Catch::Approx(0.0f));
}

TEST_CASE("moveSystem keeps facing still when a ground unit is stopped at a cliff") {
    World world;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{1.999f, 1.2f};
    mover(world, id)->target = Vector2{6.0f, 1.2f};
    mover(world, id)->speed = 10.0f;
    mover(world, id)->facing = Vector2{1.0f, 0.0f};
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    REQUIRE(setHeight(grid, 2, 1, 3));

    for (int i = 0; i < 8; ++i) {
        moveSystem(world, grid, 1.0f / 60.0f);
    }

    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, transform(world, id)->position, cellX, cellY));
    REQUIRE(cellX == 1);
    REQUIRE(mover(world, id)->facing.x == Catch::Approx(1.0f));
    REQUIRE(mover(world, id)->facing.y == Catch::Approx(0.0f));
}

TEST_CASE("an order steps around a unit and through the cell once it leaves") {
    Grid grid;
    REQUIRE(makeGrid(grid, 5, 3));
    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = cellCenter(grid, 0, 1);
    mover(world, walker)->order = cellCenter(grid, 4, 1);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 0.0f;
    mover(world, walker)->radius = 0.2f;
    transform(world, blocker)->position = cellCenter(grid, 2, 1);

    moveSystem(world, grid, 1.0f);
    int stepX = -1;
    int stepY = -1;
    REQUIRE(worldToCell(grid, mover(world, walker)->target, stepX, stepY));
    REQUIRE((stepX != 2 || stepY != 1));
    for (const Vector2& point : mover(world, walker)->waypoints) {
        REQUIRE(worldToCell(grid, point, stepX, stepY));
        REQUIRE((stepX != 2 || stepY != 1));
    }

    transform(world, blocker)->position = cellCenter(grid, 2, 0);
    moveSystem(world, grid, 1.0f);
    REQUIRE(mover(world, walker)->target.x == Catch::Approx(cellCenter(grid, 1, 1).x));
    REQUIRE(mover(world, walker)->target.y == Catch::Approx(cellCenter(grid, 1, 1).y));
}

TEST_CASE("a ramp passage works up and down, and a sealed ramp gives up") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 3));
    for (int y = 0; y < grid.height; ++y) {
        REQUIRE(setHeight(grid, 1, y, 2));
    }
    REQUIRE(setRamp(grid, 1, 1, 1));
    REQUIRE(setPlateau(grid, 2, 0, 1));
    REQUIRE(setPlateau(grid, 2, 1, 1));
    REQUIRE(setPlateau(grid, 2, 2, 1));

    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = cellCenter(grid, 1, 1);
    mover(world, walker)->order = cellCenter(grid, 2, 0);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 8.0f;
    mover(world, walker)->radius = 0.2f;
    transform(world, blocker)->position = cellCenter(grid, 2, 1);

    for (int i = 0; i < 40; ++i) {
        moveSystem(world, grid, 0.05f);
    }
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
    REQUIRE(cellX == 2);
    REQUIRE(cellY == 0);
    REQUIRE(mover(world, walker)->ordered);

    mover(world, walker)->order = cellCenter(grid, 0, 1);
    mover(world, walker)->blockedFor = 0.0f;
    for (int i = 0; i < 40; ++i) {
        moveSystem(world, grid, 0.05f);
    }
    REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
    REQUIRE(cellX == 0);
    REQUIRE(cellY == 1);

    transform(world, walker)->position = cellCenter(grid, 1, 1);
    mover(world, walker)->order = cellCenter(grid, 3, 1);
    mover(world, walker)->ordered = true;
    mover(world, walker)->blockedFor = 0.0f;
    const Entity sideA = create(world);
    const Entity sideB = create(world);
    transform(world, sideA)->position = cellCenter(grid, 2, 0);
    transform(world, sideB)->position = cellCenter(grid, 2, 2);
    moveSystem(world, grid, 0.6f);
    moveSystem(world, grid, 0.5f);
    REQUIRE_FALSE(mover(world, walker)->ordered);
    REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
    REQUIRE(cellX == 1);
}

TEST_CASE("a ramp exit waits at the cell center before the diagonal") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 3));
    for (int y = 0; y < grid.height; ++y) {
        REQUIRE(setHeight(grid, 1, y, 2));
    }
    REQUIRE(setRamp(grid, 1, 1, 1));
    REQUIRE(setPlateau(grid, 2, 0, 1));
    REQUIRE(setPlateau(grid, 2, 1, 1));
    REQUIRE(setPlateau(grid, 2, 2, 1));

    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = Vector2{1.05f, 1.8f};
    mover(world, walker)->order = cellCenter(grid, 2, 0);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 4.0f;
    mover(world, walker)->radius = 0.2f;
    transform(world, blocker)->position = cellCenter(grid, 2, 1);

    const Vector2 center = cellCenter(grid, 1, 1);
    Vector2 previousDelta{};
    bool hasDelta = false;
    int turns = 0;
    bool crossedCenter = false;
    for (int i = 0; i < 80; ++i) {
        const Vector2 before = transform(world, walker)->position;
        moveSystem(world, grid, 0.05f);
        const Vector2 delta = sub(transform(world, walker)->position, before);
        if (!crossedCenter && length(delta) > 0.001f) {
            const Vector2 towardCenter = sub(center, before);
            const float toward = delta.x * towardCenter.x + delta.y * towardCenter.y;
            REQUIRE(toward > 0.0f);
        }
        if (length(delta) > 0.001f) {
            if (hasDelta) {
                const float alignment = delta.x * previousDelta.x + delta.y * previousDelta.y;
                if (alignment <= 0.0f) {
                    turns += 1;
                }
            }
            previousDelta = delta;
            hasDelta = true;
        }
        if (distance(transform(world, walker)->position, center) <= grid.cellSize * 0.05f) {
            crossedCenter = true;
        }
    }
    REQUIRE(crossedCenter);
    REQUIRE(turns <= 1);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
    REQUIRE(cellX == 2);
    REQUIRE(cellY == 0);
}

TEST_CASE("a ramp exit returns to the center when already beside the far gap") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 3));
    for (int y = 0; y < grid.height; ++y) {
        REQUIRE(setHeight(grid, 1, y, 2));
    }
    REQUIRE(setRamp(grid, 1, 1, 1));
    REQUIRE(setPlateau(grid, 2, 0, 1));
    REQUIRE(setPlateau(grid, 2, 1, 1));
    REQUIRE(setPlateau(grid, 2, 2, 1));

    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = Vector2{1.85f, 1.7f};
    mover(world, walker)->order = cellCenter(grid, 2, 0);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 4.0f;
    mover(world, walker)->radius = 0.2f;
    transform(world, blocker)->position = cellCenter(grid, 2, 1);

    const Vector2 center = cellCenter(grid, 1, 1);
    Vector2 previousDelta{};
    bool hasDelta = false;
    int turns = 0;
    bool crossedCenter = false;
    for (int i = 0; i < 80; ++i) {
        const Vector2 before = transform(world, walker)->position;
        moveSystem(world, grid, 0.05f);
        const Vector2 delta = sub(transform(world, walker)->position, before);
        if (!crossedCenter && length(delta) > 0.001f) {
            const Vector2 towardCenter = sub(center, before);
            const float toward = delta.x * towardCenter.x + delta.y * towardCenter.y;
            REQUIRE(toward > 0.0f);
        }
        if (length(delta) > 0.001f) {
            if (hasDelta) {
                const float alignment = delta.x * previousDelta.x + delta.y * previousDelta.y;
                if (alignment <= 0.0f) {
                    turns += 1;
                }
            }
            previousDelta = delta;
            hasDelta = true;
        }
        if (distance(transform(world, walker)->position, center) <= grid.cellSize * 0.05f) {
            crossedCenter = true;
        }
    }
    REQUIRE(crossedCenter);
    REQUIRE(turns <= 1);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
    REQUIRE(cellX == 2);
    REQUIRE(cellY == 0);
}

TEST_CASE("a blocked ramp exit does not orbit against the straight cell") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 3));
    for (int y = 0; y < grid.height; ++y) {
        REQUIRE(setHeight(grid, 1, y, 2));
    }
    REQUIRE(setRamp(grid, 1, 1, 1));
    REQUIRE(setPlateau(grid, 2, 0, 1));
    REQUIRE(setPlateau(grid, 2, 1, 1));
    REQUIRE(setPlateau(grid, 2, 2, 1));

    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = Vector2{1.92f, 1.35f};
    mover(world, walker)->order = cellCenter(grid, 2, 0);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 4.0f;
    mover(world, walker)->radius = 0.2f;
    transform(world, blocker)->position = cellCenter(grid, 2, 1);

    Vector2 previousDelta{};
    bool hasDelta = false;
    int turns = 0;
    for (int i = 0; i < 80; ++i) {
        const Vector2 before = transform(world, walker)->position;
        moveSystem(world, grid, 0.05f);
        const Vector2 delta = sub(transform(world, walker)->position, before);
        if (length(delta) <= 0.001f) {
            continue;
        }
        if (hasDelta) {
            const float alignment = delta.x * previousDelta.x + delta.y * previousDelta.y;
            if (alignment <= 0.0f) {
                turns += 1;
            }
        }
        previousDelta = delta;
        hasDelta = true;
    }
    REQUIRE(turns <= 2);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
    REQUIRE(cellX == 2);
    REQUIRE(cellY == 0);
}

TEST_CASE("an open ramp is crossed straight through the center") {
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 3));
    for (int y = 0; y < grid.height; ++y) {
        REQUIRE(setHeight(grid, 1, y, 2));
    }
    REQUIRE(setRamp(grid, 1, 1, 1));
    REQUIRE(setPlateau(grid, 2, 0, 1));
    REQUIRE(setPlateau(grid, 2, 1, 1));
    REQUIRE(setPlateau(grid, 2, 2, 1));

    World world;
    const Entity walker = create(world);
    transform(world, walker)->position = Vector2{1.05f, 1.8f};
    mover(world, walker)->order = cellCenter(grid, 2, 0);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 4.0f;
    mover(world, walker)->radius = 0.2f;

    const Vector2 center = cellCenter(grid, 1, 1);
    bool crossedCenter = false;
    bool leftThroughStraight = false;
    for (int i = 0; i < 80; ++i) {
        const Vector2 before = transform(world, walker)->position;
        int beforeX = -1;
        int beforeY = -1;
        REQUIRE(worldToCell(grid, before, beforeX, beforeY));
        moveSystem(world, grid, 0.05f);
        int cellX = -1;
        int cellY = -1;
        REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
        if (beforeX == 1 && beforeY == 1 && !(cellX == 1 && cellY == 1)) {
            REQUIRE(crossedCenter);
            REQUIRE(cellX == 2);
            REQUIRE(cellY == 1);
            leftThroughStraight = true;
        }
        if (beforeX == 1 && beforeY == 1 &&
            distance(transform(world, walker)->position, center) <= grid.cellSize * 0.05f) {
            crossedCenter = true;
        }
    }
    REQUIRE(crossedCenter);
    REQUIRE(leftThroughStraight);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
    REQUIRE(cellX == 2);
    REQUIRE(cellY == 0);
}

TEST_CASE("an equal passage keeps the side already chosen") {
    Grid grid;
    REQUIRE(makeGrid(grid, 5, 3));
    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = cellCenter(grid, 1, 1);
    mover(world, walker)->order = cellCenter(grid, 4, 1);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 0.0f;
    mover(world, walker)->radius = 0.2f;
    transform(world, blocker)->position = cellCenter(grid, 2, 1);

    moveSystem(world, grid, 1.0f);
    const Vector2 chosen = mover(world, walker)->target;
    transform(world, walker)->position.y += 0.2f;
    moveSystem(world, grid, 1.0f);
    REQUIRE(mover(world, walker)->target.x == Catch::Approx(chosen.x));
    REQUIRE(mover(world, walker)->target.y == Catch::Approx(chosen.y));
}

TEST_CASE("a unit blocking the way sends the walker back to his cell after one second") {
    Grid grid;
    REQUIRE(makeGrid(grid, 5, 1));
    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = cellCenter(grid, 1, 0);
    mover(world, walker)->order = cellCenter(grid, 4, 0);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 10.0f;
    mover(world, walker)->radius = 0.2f;
    transform(world, blocker)->position = cellCenter(grid, 2, 0);

    moveSystem(world, grid, 0.6f);
    REQUIRE(mover(world, walker)->ordered);
    REQUIRE(transform(world, walker)->position.x > cellCenter(grid, 1, 0).x);
    moveSystem(world, grid, 0.4f);
    REQUIRE_FALSE(mover(world, walker)->ordered);
    REQUIRE(transform(world, walker)->position.x == Catch::Approx(1.999f));

    transform(world, blocker)->position = cellCenter(grid, 0, 0);
    moveSystem(world, grid, 1.0f);
    REQUIRE_FALSE(mover(world, walker)->ordered);
    REQUIRE(transform(world, walker)->position.x == Catch::Approx(cellCenter(grid, 1, 0).x));
    REQUIRE(transform(world, walker)->position.y == Catch::Approx(cellCenter(grid, 1, 0).y));
}

TEST_CASE("a slide off the blocked cell does not reset the wait") {
    Grid grid;
    REQUIRE(makeGrid(grid, 5, 1));
    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = cellCenter(grid, 1, 0);
    mover(world, walker)->order = cellCenter(grid, 4, 0);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 10.0f;
    mover(world, walker)->radius = 0.2f;
    transform(world, blocker)->position = cellCenter(grid, 2, 0);

    moveSystem(world, grid, 0.4f);
    REQUIRE(mover(world, walker)->ordered);
    REQUIRE(mover(world, walker)->blockedFor == Catch::Approx(0.4f));

    transform(world, walker)->position = cellCenter(grid, 0, 0);
    moveSystem(world, grid, 0.7f);
    REQUIRE_FALSE(mover(world, walker)->ordered);
}

TEST_CASE("a blocked order continues when the cell opens within one second") {
    Grid grid;
    REQUIRE(makeGrid(grid, 5, 3));
    for (int x = 0; x < grid.width; ++x) {
        REQUIRE(setHeight(grid, x, 0, 2));
        REQUIRE(setHeight(grid, x, 2, 2));
    }
    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = Vector2{1.8f, 1.5f};
    mover(world, walker)->order = cellCenter(grid, 4, 1);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 0.0f;
    mover(world, walker)->radius = 0.2f;
    transform(world, blocker)->position = cellCenter(grid, 2, 1);

    moveSystem(world, grid, 0.4f);
    REQUIRE(mover(world, walker)->ordered);
    transform(world, blocker)->position = cellCenter(grid, 2, 0);
    moveSystem(world, grid, 0.1f);
    REQUIRE(mover(world, walker)->ordered);
    REQUIRE(mover(world, walker)->target.x == Catch::Approx(cellCenter(grid, 2, 1).x));
    REQUIRE(mover(world, walker)->target.y == Catch::Approx(cellCenter(grid, 2, 1).y));
}

TEST_CASE("a unit pressed into an occupied cell steps back to its own cell after one second") {
    Grid grid;
    REQUIRE(makeGrid(grid, 6, 3));
    grid.cellSize = 4.0f;
    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = cellCenter(grid, 4, 1);
    mover(world, walker)->order = cellCenter(grid, 2, 1);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 10.0f;
    mover(world, walker)->radius = 2.0f;
    transform(world, blocker)->position = cellCenter(grid, 2, 1);
    mover(world, blocker)->radius = 2.0f;

    for (int i = 0; i < 40; ++i) {
        moveSystem(world, grid, 1.0f / 60.0f);
    }
    REQUIRE(mover(world, walker)->ordered);
    REQUIRE(transform(world, walker)->position.x < cellCenter(grid, 3, 1).x);

    for (int i = 0; i < 80; ++i) {
        moveSystem(world, grid, 1.0f / 60.0f);
    }
    REQUIRE_FALSE(mover(world, walker)->ordered);
    REQUIRE(transform(world, walker)->position.x == Catch::Approx(cellCenter(grid, 3, 1).x));
    REQUIRE(transform(world, walker)->position.y == Catch::Approx(cellCenter(grid, 3, 1).y));

    transform(world, blocker)->position = cellCenter(grid, 0, 0);
    moveSystem(world, grid, 1.0f);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
    REQUIRE(cellX == 3);
    REQUIRE(cellY == 1);
}

TEST_CASE("every neighbor steps back from an occupied cell after one second") {
    const int neighbors[8][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    for (const auto& neighbor : neighbors) {
        Grid grid;
        REQUIRE(makeGrid(grid, 9, 9));
        grid.cellSize = 4.0f;
        World world;
        const Entity walker = create(world);
        const Entity blocker = create(world);
        transform(world, blocker)->position = cellCenter(grid, 4, 4);
        mover(world, blocker)->radius = 2.0f;
        transform(world, walker)->position = cellCenter(grid, 4 + neighbor[0] * 3, 4 + neighbor[1] * 3);
        mover(world, walker)->order = cellCenter(grid, 4, 4);
        mover(world, walker)->ordered = true;
        mover(world, walker)->speed = 10.0f;
        mover(world, walker)->radius = 2.0f;

        bool entered = false;
        for (int i = 0; i < 240; ++i) {
            moveSystem(world, grid, 1.0f / 60.0f);
            int cx = 0;
            int cy = 0;
            if (worldToCell(grid, transform(world, walker)->position, cx, cy) && cx == 4 && cy == 4) {
                entered = true;
            }
        }
        int cellX = -1;
        int cellY = -1;
        REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
        const bool tookTheCell = cellX == 4 && cellY == 4;
        INFO("dir " << neighbor[0] << "," << neighbor[1] << " ended " << cellX << "," << cellY);
        REQUIRE_FALSE(mover(world, walker)->ordered);
        REQUIRE_FALSE(entered);
        REQUIRE_FALSE(tookTheCell);
        REQUIRE(cellX == 4 + neighbor[0]);
        REQUIRE(cellY == 4 + neighbor[1]);
        REQUIRE(transform(world, walker)->position.x == Catch::Approx(cellCenter(grid, cellX, cellY).x));
        REQUIRE(transform(world, walker)->position.y == Catch::Approx(cellCenter(grid, cellX, cellY).y));

        transform(world, blocker)->position = cellCenter(grid, 0, 0);
        moveSystem(world, grid, 1.0f);
        REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
        REQUIRE(cellX == 4 + neighbor[0]);
        REQUIRE(cellY == 4 + neighbor[1]);
    }
}

TEST_CASE("a diagonal press into an occupied cell steps back after one second") {
    Grid grid;
    REQUIRE(makeGrid(grid, 6, 4));
    grid.cellSize = 4.0f;
    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = cellCenter(grid, 4, 3);
    mover(world, walker)->order = cellCenter(grid, 2, 1);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 10.0f;
    mover(world, walker)->radius = 2.0f;
    transform(world, blocker)->position = cellCenter(grid, 2, 1);
    mover(world, blocker)->radius = 2.0f;

    for (int i = 0; i < 180; ++i) {
        moveSystem(world, grid, 1.0f / 60.0f);
    }
    REQUIRE_FALSE(mover(world, walker)->ordered);
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
    REQUIRE((cellX != 2 || cellY != 1));
    REQUIRE(transform(world, walker)->position.x == Catch::Approx(cellCenter(grid, cellX, cellY).x));
    REQUIRE(transform(world, walker)->position.y == Catch::Approx(cellCenter(grid, cellX, cellY).y));

    transform(world, blocker)->position = cellCenter(grid, 0, 0);
    moveSystem(world, grid, 1.0f);
    REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
    REQUIRE((cellX != 2 || cellY != 1));
}

TEST_CASE("an open detour around a unit still reaches the goal") {
    Grid grid;
    REQUIRE(makeGrid(grid, 7, 3));
    grid.cellSize = 4.0f;
    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = cellCenter(grid, 0, 1);
    mover(world, walker)->order = cellCenter(grid, 5, 1);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 10.0f;
    mover(world, walker)->radius = 2.0f;
    transform(world, blocker)->position = cellCenter(grid, 2, 1);
    mover(world, blocker)->radius = 2.0f;

    for (int i = 0; i < 240; ++i) {
        moveSystem(world, grid, 1.0f / 60.0f);
    }
    int cellX = -1;
    int cellY = -1;
    REQUIRE(worldToCell(grid, transform(world, walker)->position, cellX, cellY));
    REQUIRE(cellX == 5);
    REQUIRE(cellY == 1);
}

TEST_CASE("an order keeps walking toward a block for longer than a second") {
    Grid grid;
    REQUIRE(makeGrid(grid, 5, 1));
    World world;
    const Entity walker = create(world);
    const Entity blocker = create(world);
    transform(world, walker)->position = cellCenter(grid, 0, 0);
    mover(world, walker)->order = cellCenter(grid, 4, 0);
    mover(world, walker)->ordered = true;
    mover(world, walker)->speed = 0.2f;
    mover(world, walker)->radius = 0.2f;
    transform(world, blocker)->position = cellCenter(grid, 2, 0);

    moveSystem(world, grid, 1.5f);
    REQUIRE(mover(world, walker)->ordered);
    REQUIRE(transform(world, walker)->position.x == Catch::Approx(0.8f));
}
