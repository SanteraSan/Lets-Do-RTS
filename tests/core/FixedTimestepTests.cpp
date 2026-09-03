#include "core/FixedTimestep.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("consumeStep is false until a full delta is accumulated") {
    FixedTimestep step;
    step.delta = 0.1f;
    addFrameTime(step, 0.1f);
    REQUIRE(consumeStep(step));
    REQUIRE_FALSE(consumeStep(step));
}

TEST_CASE("consumeStep runs once per full delta and keeps the remainder") {
    FixedTimestep step;
    step.delta = 0.1f;
    addFrameTime(step, 0.25f);

    int ticks = 0;
    while (consumeStep(step)) {
        ticks += 1;
    }

    REQUIRE(ticks == 2);
    REQUIRE(step.accumulator == Catch::Approx(0.05f));
}

TEST_CASE("addFrameTime ignores non-positive durations") {
    FixedTimestep step;
    addFrameTime(step, 0.0f);
    addFrameTime(step, -0.5f);
    REQUIRE(step.accumulator == 0.0f);
    REQUIRE_FALSE(consumeStep(step));
}

TEST_CASE("consumeStep is false when delta is not positive") {
    FixedTimestep step;
    step.delta = 0.0f;
    step.accumulator = 1.0f;
    REQUIRE_FALSE(consumeStep(step));
}
