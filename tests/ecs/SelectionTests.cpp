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
    applyInput(world, selection, clickOnUnit, camera, Grid{});
    REQUIRE(selection.entity == id);

    InputFrame clickEmpty;
    clickEmpty.leftClick = true;
    clickEmpty.mouse = worldToScreen(Vector2{50.0f, 50.0f}, camera);
    applyInput(world, selection, clickEmpty, camera, Grid{});
    REQUIRE(selection.entity == kInvalidEntity);
}

TEST_CASE("right click on open ground snaps the target to the cell center") {
    World world;
    Selection selection;
    const Entity id = create(world);
    selection.entity = id;
    transform(world, id)->position = Vector2{0.2f, 4.2f};
    const Camera camera{};
    Grid grid;
    REQUIRE(makeGrid(grid, 40, 10));

    InputFrame order;
    order.rightClick = true;
    order.mouse = worldToScreen(Vector2{30.2f, 4.2f}, camera);
    applyInput(world, selection, order, camera, grid);
    REQUIRE(mover(world, id)->target.x == Catch::Approx(30.5f));
    REQUIRE(mover(world, id)->target.y == Catch::Approx(4.5f));
}

TEST_CASE("right click past a cliff stops a ground unit against the cliff") {
    World world;
    Selection selection;
    const Entity id = create(world);
    selection.entity = id;
    transform(world, id)->position = Vector2{0.2f, 1.2f};
    mover(world, id)->target = Vector2{0.0f, 0.0f};
    mover(world, id)->radius = 0.2f;
    const Camera camera{};
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 4));
    for (int row = 0; row < grid.height; ++row) {
        REQUIRE(setHeight(grid, 3, row, 2));
    }

    InputFrame order;
    order.rightClick = true;
    order.mouse = worldToScreen(Vector2{6.2f, 1.2f}, camera);
    applyInput(world, selection, order, camera, grid);
    REQUIRE(mover(world, id)->target.x == Catch::Approx(2.8f));
    REQUIRE(mover(world, id)->target.y == Catch::Approx(1.5f));
}

TEST_CASE("right click on a cliff sends an air unit to the cell center") {
    World world;
    Selection selection;
    const Entity id = create(world);
    selection.entity = id;
    transform(world, id)->position = Vector2{0.2f, 2.2f};
    transform(world, id)->layer = NavLayer::Air;
    mover(world, id)->target = Vector2{0.0f, 0.0f};
    const Camera camera{};
    Grid grid;
    REQUIRE(makeGrid(grid, 8, 8));
    REQUIRE(setHeight(grid, 3, 2, 2));

    InputFrame order;
    order.rightClick = true;
    order.mouse = worldToScreen(Vector2{3.1f, 2.1f}, camera);
    applyInput(world, selection, order, camera, grid);
    REQUIRE(mover(world, id)->target.x == Catch::Approx(3.5f));
    REQUIRE(mover(world, id)->target.y == Catch::Approx(2.5f));
}

TEST_CASE("right click outside the grid leaves the target") {
    World world;
    Selection selection;
    const Entity id = create(world);
    selection.entity = id;
    mover(world, id)->target = Vector2{1.0f, 1.0f};
    const Camera camera{};
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 4));

    InputFrame order;
    order.rightClick = true;
    order.mouse = worldToScreen(Vector2{20.0f, 20.0f}, camera);
    applyInput(world, selection, order, camera, grid);
    REQUIRE(mover(world, id)->target.x == Catch::Approx(1.0f));
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
    applyInput(world, selection, order, camera, Grid{});
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
    applyInput(world, selection, walk, camera, Grid{});
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
    applyInput(world, selection, walk, camera, Grid{});
    REQUIRE(mover(world, id)->speed == Catch::Approx(kWalkSpeed));

    InputFrame run;
    run.keyRun = true;
    applyInput(world, selection, run, camera, Grid{});
    REQUIRE(mover(world, id)->speed == Catch::Approx(kRunSpeed));
}
