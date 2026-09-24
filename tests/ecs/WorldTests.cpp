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
