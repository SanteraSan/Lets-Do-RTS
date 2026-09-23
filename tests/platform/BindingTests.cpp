#include "platform/Binding.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("default bindings map left click, right click, 1 and 2") {
    InputAction action = InputAction::Run;
    REQUIRE(findAction(kDefaultBindings, kDefaultBindingCount, true, SDL_BUTTON_LEFT, action));
    REQUIRE(action == InputAction::Select);
    REQUIRE(findAction(kDefaultBindings, kDefaultBindingCount, true, SDL_BUTTON_RIGHT, action));
    REQUIRE(action == InputAction::Order);
    REQUIRE(findAction(kDefaultBindings, kDefaultBindingCount, false, SDLK_1, action));
    REQUIRE(action == InputAction::Walk);
    REQUIRE(findAction(kDefaultBindings, kDefaultBindingCount, false, SDLK_2, action));
    REQUIRE(action == InputAction::Run);
}

TEST_CASE("a custom row rebinds a key without changing the action names") {
    const Binding swapped[] = {{false, 99, InputAction::Run}};
    InputFrame input;
    InputAction action = InputAction::Select;
    REQUIRE(findAction(swapped, 1, false, 99, action));
    REQUIRE(action == InputAction::Run);
    raiseAction(input, action);
    REQUIRE(input.keyRun);
    REQUIRE_FALSE(input.keyWalk);
}

TEST_CASE("findAction ignores an unknown code") {
    InputAction action = InputAction::Select;
    REQUIRE_FALSE(findAction(kDefaultBindings, kDefaultBindingCount, false, SDLK_9, action));
    REQUIRE(action == InputAction::Select);
}

TEST_CASE("findAction rejects a null table, empty count, and a negative count") {
    InputAction action = InputAction::Walk;
    REQUIRE_FALSE(findAction(nullptr, 1, false, SDLK_1, action));
    REQUIRE_FALSE(findAction(kDefaultBindings, 0, false, SDLK_1, action));
    REQUIRE_FALSE(findAction(kDefaultBindings, -1, false, SDLK_1, action));
    REQUIRE(action == InputAction::Walk);
}

TEST_CASE("findAction keeps the first row when two rows share a code") {
    const Binding rows[] = {
        {false, 99, InputAction::Walk},
        {false, 99, InputAction::Run},
    };
    InputAction action = InputAction::Select;
    REQUIRE(findAction(rows, 2, false, 99, action));
    REQUIRE(action == InputAction::Walk);
}

TEST_CASE("mouse and key codes do not collide when the number matches") {
    const Binding rows[] = {
        {true, 1, InputAction::Select},
        {false, 1, InputAction::Walk},
    };
    InputAction action = InputAction::Run;
    REQUIRE(findAction(rows, 2, false, 1, action));
    REQUIRE(action == InputAction::Walk);
    REQUIRE(findAction(rows, 2, true, 1, action));
    REQUIRE(action == InputAction::Select);
}
