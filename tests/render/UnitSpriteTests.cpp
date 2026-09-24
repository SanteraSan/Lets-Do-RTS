#include "render/UnitSprite.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("unit sprite is a 32 by 32 image") {
    const SpriteImage sprite = makeUnitSprite();
    REQUIRE(sprite.width == 32);
    REQUIRE(sprite.height == 32);
    REQUIRE(sprite.rgba.size() == 32 * 32 * 4);
}

TEST_CASE("unit sprite center is opaque and the corner is empty") {
    const SpriteImage sprite = makeUnitSprite();
    const std::size_t center = static_cast<std::size_t>((16 * 32 + 16) * 4 + 3);
    const std::size_t corner = 3;
    REQUIRE(sprite.rgba[center] == 255);
    REQUIRE(sprite.rgba[corner] == 0);
}
