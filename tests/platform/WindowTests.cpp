#include "platform/Window.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("isValidWindowDesc accepts a positive size and title") {
    REQUIRE(isValidWindowDesc({"Lets Do RTS", 1280, 720}));
}

TEST_CASE("isValidWindowDesc rejects a null title") {
    REQUIRE_FALSE(isValidWindowDesc({nullptr, 1280, 720}));
}

TEST_CASE("isValidWindowDesc rejects a non-positive size") {
    REQUIRE_FALSE(isValidWindowDesc({"Lets Do RTS", 0, 720}));
    REQUIRE_FALSE(isValidWindowDesc({"Lets Do RTS", 1280, -1}));
}
