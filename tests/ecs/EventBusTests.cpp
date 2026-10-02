#include "ecs/EventBus.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("a subscriber receives a published event") {
    EventBus<int> bus;
    int received = 0;
    int count = 0;
    subscribe<int>(bus, [&](const int& value) {
        received = value;
        count += 1;
    });
    publish(bus, 7);
    REQUIRE(received == 7);
    REQUIRE(count == 1);
}
