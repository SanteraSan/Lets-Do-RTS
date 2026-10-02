#include "ecs/systems/Selection.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("hitUnit returns the entity whose cube contains the click") {
    World world;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{10.0f, 0.0f};
    mover(world, id)->radius = 2.0f;
    const Camera camera{};
    const ScreenPoint click = worldToScreen(Vector2{11.0f, 0.0f}, camera);
    REQUIRE(hitUnit(world, click, camera, Grid{}) == id);
}

TEST_CASE("hitUnit misses when the click is outside every cube") {
    World world;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{10.0f, 0.0f};
    mover(world, id)->radius = 2.0f;
    const Camera camera{};
    const ScreenPoint click = worldToScreen(Vector2{20.0f, 0.0f}, camera);
    REQUIRE(hitUnit(world, click, camera, Grid{}) == kInvalidEntity);
}

TEST_CASE("hitUnit prefers the later entity when cubes overlap") {
    World world;
    const Entity first = create(world);
    const Entity second = create(world);
    transform(world, first)->position = Vector2{0.0f, 0.0f};
    mover(world, first)->radius = 5.0f;
    transform(world, second)->position = Vector2{0.0f, 0.0f};
    mover(world, second)->radius = 5.0f;
    const Camera camera{};
    const ScreenPoint click = worldToScreen(Vector2{0.0f, 0.0f}, camera);
    REQUIRE(hitUnit(world, click, camera, Grid{}) == second);
}

TEST_CASE("hitUnit selects an air unit on its raised cube") {
    World world;
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 4));
    grid.cellSize = 4.0f;
    const Entity id = create(world);
    transform(world, id)->position = cellCenter(grid, 1, 1);
    transform(world, id)->layer = NavLayer::Air;
    mover(world, id)->radius = 2.0f;
    const Camera camera{};
    const ScreenPoint onGround = worldToScreen(transform(world, id)->position, camera);
    const ScreenPoint onCube = worldToScreen(transform(world, id)->position, standLift(grid, transform(world, id)->position, NavLayer::Air), camera);
    REQUIRE(hitUnit(world, onGround, camera, grid) == kInvalidEntity);
    REQUIRE(hitUnit(world, onCube, camera, grid) == id);
}

TEST_CASE("hitUnit selects a ground soldier inside the sprite") {
    World world;
    Grid grid;
    grid.cellSize = 4.0f;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{0.0f, 0.0f};
    mover(world, id)->radius = 2.0f;
    const Camera camera{};
    const SpriteScreen frame =
        groundSpriteScreen(transform(world, id)->position, mover(world, id)->facing, 0.0f, grid.cellSize, 1, 100, camera);
    const ScreenPoint body{frame.feet.x, frame.feet.y - frame.pixelHeight / 2};
    REQUIRE(hitUnit(world, body, camera, grid, 1, 100) == id);
}

TEST_CASE("hitUnit misses a ground soldier beside the sprite") {
    World world;
    Grid grid;
    grid.cellSize = 4.0f;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{0.0f, 0.0f};
    mover(world, id)->radius = 2.0f;
    const Camera camera{};
    const SpriteScreen frame =
        groundSpriteScreen(transform(world, id)->position, mover(world, id)->facing, 0.0f, grid.cellSize, 1, 100, camera);
    const ScreenPoint aside{frame.feet.x + frame.pixelWidth + 8, frame.feet.y - frame.pixelHeight / 2};
    REQUIRE(hitUnit(world, aside, camera, grid, 1, 100) == kInvalidEntity);
}

TEST_CASE("hitUnit keeps the boat cube when the soldier picture is set") {
    World world;
    Grid grid;
    grid.cellSize = 4.0f;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{0.0f, 0.0f};
    transform(world, id)->layer = NavLayer::Surface;
    mover(world, id)->radius = 2.0f;
    const Camera camera{};
    const ScreenPoint onCube = worldToScreen(Vector2{1.0f, 0.0f}, camera);
    REQUIRE(hitUnit(world, onCube, camera, grid, 1, 100) == id);
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

TEST_CASE("a left click publishes a selection and the subscriber selects the unit") {
    World world;
    Selection selection;
    const Entity id = create(world);
    transform(world, id)->position = Vector2{0.0f, 0.0f};
    mover(world, id)->radius = 2.0f;
    const Camera camera{};

    EventBus<OrderEvent> orders;
    EventBus<SelectEvent> selections;
    SelectEvent seen{};
    int count = 0;
    subscribe<SelectEvent>(selections, [&](const SelectEvent& event) {
        seen = event;
        count += 1;
        applySelection(selection, event);
    });

    InputFrame click;
    click.leftClick = true;
    click.mouse = worldToScreen(Vector2{0.0f, 0.0f}, camera);
    EventBus<SpeedEvent> speeds;
    applyInput(world, selection, click, camera, Grid{}, orders, selections, speeds);

    REQUIRE(count == 1);
    REQUIRE(seen.unit == id);
    REQUIRE(selection.entity == id);
}

TEST_CASE("a right click publishes an order and the subscriber sends the unit") {
    World world;
    Selection selection;
    const Entity id = create(world);
    selection.entity = id;
    transform(world, id)->position = Vector2{0.2f, 0.2f};
    const Camera camera{};
    Grid grid;
    REQUIRE(makeGrid(grid, 4, 4));

    EventBus<OrderEvent> orders;
    OrderEvent seen{};
    int count = 0;
    subscribe<OrderEvent>(orders, [&](const OrderEvent& order) {
        seen = order;
        count += 1;
        issueOrder(world, grid, order);
    });

    InputFrame click;
    click.rightClick = true;
    click.mouse = worldToScreen(cellCenter(grid, 2, 1), camera);
    EventBus<SelectEvent> selections;
    EventBus<SpeedEvent> speeds;
    applyInput(world, selection, click, camera, grid, orders, selections, speeds);

    REQUIRE(count == 1);
    REQUIRE(seen.unit == id);
    REQUIRE(mover(world, id)->ordered);
    REQUIRE(mover(world, id)->order.x == Catch::Approx(cellCenter(grid, 2, 1).x));
    REQUIRE(mover(world, id)->order.y == Catch::Approx(cellCenter(grid, 2, 1).y));
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
    REQUIRE_FALSE(mover(world, id)->waypoints.empty());
    REQUIRE(mover(world, id)->waypoints.back().x == Catch::Approx(30.5f));
    REQUIRE(mover(world, id)->waypoints.back().y == Catch::Approx(4.5f));
}

TEST_CASE("right click on a raised plateau orders that plateau cell") {
    World world;
    Selection selection;
    const Entity id = create(world);
    selection.entity = id;
    transform(world, id)->position = Vector2{0.2f, 0.2f};
    const Camera camera{};
    Grid grid;
    REQUIRE(makeGrid(grid, 6, 6));
    grid.cellSize = 4.0f;
    REQUIRE(setRamp(grid, 2, 3, 0));
    REQUIRE(setPlateau(grid, 3, 3, 1));
    const Vector2 center = cellCenter(grid, 3, 3);
    const float lift = surfaceLift(1, grid.cellSize);

    InputFrame order;
    order.rightClick = true;
    order.mouse = worldToScreen(center, lift, camera);
    applyInput(world, selection, order, camera, grid);
    REQUIRE(mover(world, id)->order.x == Catch::Approx(center.x));
    REQUIRE(mover(world, id)->order.y == Catch::Approx(center.y));
    REQUIRE_FALSE(mover(world, id)->waypoints.empty());
    REQUIRE(mover(world, id)->waypoints.back().x == Catch::Approx(center.x));
    REQUIRE(mover(world, id)->waypoints.back().y == Catch::Approx(center.y));
    const Vector2 flat = screenToWorld(order.mouse, camera);
    int flatX = -1;
    int flatY = -1;
    REQUIRE(worldToCell(grid, flat, flatX, flatY));
    REQUIRE((flatX != 3 || flatY != 3));
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
    REQUIRE_FALSE(mover(world, id)->waypoints.empty());
    REQUIRE(mover(world, id)->waypoints.back().x == Catch::Approx(2.8f));
    REQUIRE(mover(world, id)->waypoints.back().y == Catch::Approx(1.5f));
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
    REQUIRE_FALSE(mover(world, id)->waypoints.empty());
    REQUIRE(mover(world, id)->waypoints.back().x == Catch::Approx(3.5f));
    REQUIRE(mover(world, id)->waypoints.back().y == Catch::Approx(2.5f));
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

TEST_CASE("a speed key publishes a speed and the subscriber sets it") {
    World world;
    Selection selection;
    const Entity id = create(world);
    selection.entity = id;
    mover(world, id)->speed = 10.0f;
    const Camera camera{};

    EventBus<OrderEvent> orders;
    EventBus<SelectEvent> selections;
    EventBus<SpeedEvent> speeds;
    SpeedEvent seen{};
    int count = 0;
    subscribe<SpeedEvent>(speeds, [&](const SpeedEvent& event) {
        seen = event;
        count += 1;
        applySpeed(world, event);
    });

    InputFrame walk;
    walk.keyWalk = true;
    applyInput(world, selection, walk, camera, Grid{}, orders, selections, speeds);

    REQUIRE(count == 1);
    REQUIRE(seen.unit == id);
    REQUIRE(seen.speed == Catch::Approx(kWalkSpeed));
    REQUIRE(mover(world, id)->speed == Catch::Approx(kWalkSpeed));
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
