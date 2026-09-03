#include "ecs/systems/Selection.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("hitUnit returns the entity whose circle contains the point") {
    World world;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{10.0f, 0.0f};
    mover(world, id)->radius = 2.0f;
    REQUIRE(hitUnit(world, Vector2{11.0f, 0.0f}) == id);
}

TEST_CASE("hitUnit misses when the point is outside every radius") {
    World world;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{10.0f, 0.0f};
    mover(world, id)->radius = 2.0f;
    REQUIRE(hitUnit(world, Vector2{20.0f, 0.0f}) == kInvalidEntity);
}

TEST_CASE("hitUnit prefers the later entity when circles overlap") {
    World world;
    const Entity first = create(world);
    const Entity second = create(world);
    transform(world, first)->position = Vector2{0.0f, 0.0f};
    mover(world, first)->radius = 5.0f;
    transform(world, second)->position = Vector2{0.0f, 0.0f};
    mover(world, second)->radius = 5.0f;
    REQUIRE(hitUnit(world, Vector2{0.0f, 0.0f}) == second);
}

TEST_CASE("left click selects a unit and click on empty space clears it") {
    World world;
    Selection selection;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{0.0f, 0.0f};
    mover(world, id)->radius = 2.0f;
    const Camera camera{};

    InputFrame clickOnUnit;
    clickOnUnit.leftClick = true;
    clickOnUnit.mouse = worldToScreen(Vector2{0.0f, 0.0f}, camera);
    applyInput(world, selection, clickOnUnit, camera);
    REQUIRE(selection.entity == id);

    InputFrame clickEmpty;
    clickEmpty.leftClick = true;
    clickEmpty.mouse = worldToScreen(Vector2{50.0f, 50.0f}, camera);
    applyInput(world, selection, clickEmpty, camera);
    REQUIRE(selection.entity == kInvalidEntity);
}

TEST_CASE("right click sets the selected unit target") {
    World world;
    Selection selection;
    const Entity id = create(world);
    selection.entity = id;
    mover(world, id)->target = Vector2{0.0f, 0.0f};
    const Camera camera{};

    InputFrame order;
    order.rightClick = true;
    order.mouse = worldToScreen(Vector2{30.0f, 4.0f}, camera);
    applyInput(world, selection, order, camera);
    REQUIRE(mover(world, id)->target.x == Catch::Approx(30.0f));
    REQUIRE(mover(world, id)->target.y == Catch::Approx(4.0f));
}

TEST_CASE("right click does nothing without a selection") {
    World world;
    Selection selection;
    const Entity id = create(world);
    mover(world, id)->target = Vector2{1.0f, 1.0f};
    const Camera camera{};

    InputFrame order;
    order.rightClick = true;
    order.mouse = worldToScreen(Vector2{30.0f, 0.0f}, camera);
    applyInput(world, selection, order, camera);
    REQUIRE(mover(world, id)->target.x == Catch::Approx(1.0f));
}

TEST_CASE("speed keys do nothing without a selection") {
    World world;
    Selection selection;
    const Entity id = create(world);
    mover(world, id)->speed = 10.0f;
    const Camera camera{};

    InputFrame walk;
    walk.keyWalk = true;
    applyInput(world, selection, walk, camera);
    REQUIRE(mover(world, id)->speed == Catch::Approx(10.0f));
}

TEST_CASE("keys 1 and 2 change selected unit speed") {
    World world;
    Selection selection;
    const Entity id = create(world);
    selection.entity = id;
    mover(world, id)->speed = 10.0f;
    const Camera camera{};

    InputFrame walk;
    walk.keyWalk = true;
    applyInput(world, selection, walk, camera);
    REQUIRE(mover(world, id)->speed == Catch::Approx(kWalkSpeed));

    InputFrame run;
    run.keyRun = true;
    applyInput(world, selection, run, camera);
    REQUIRE(mover(world, id)->speed == Catch::Approx(kRunSpeed));
}
