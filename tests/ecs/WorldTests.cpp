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
    transform(world, a)->position = Vector2{0.0f, 0.0f};
    mover(world, a)->target = Vector2{10.0f, 0.0f};
    mover(world, a)->speed = 2.0f;
    transform(world, b)->position = Vector2{0.0f, 5.0f};
    mover(world, b)->target = Vector2{10.0f, 5.0f};
    mover(world, b)->speed = 2.0f;

    moveSystem(world, 1.0f);

    REQUIRE(transform(world, a)->position.x == Catch::Approx(2.0f));
    REQUIRE(transform(world, b)->position.x == Catch::Approx(2.0f));
    REQUIRE(mover(world, a)->facing.x == Catch::Approx(1.0f));
}
