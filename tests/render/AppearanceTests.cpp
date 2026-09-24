#include "render/Appearance.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("flat ground is drawn as dark ground") {
    const TileAppearance look = tileAppearance(Tile{});
    REQUIRE(look.fill.red == 24);
    REQUIRE(look.fill.green == 36);
    REQUIRE(look.fill.blue == 44);
    REQUIRE_FALSE(look.deck);
}

TEST_CASE("a cliff is drawn lighter than ground") {
    const TileAppearance look = tileAppearance(Tile{2, false});
    REQUIRE(look.fill.red == 120);
    REQUIRE(look.fill.green == 96);
    REQUIRE(look.fill.blue == 48);
    REQUIRE_FALSE(look.deck);
}

TEST_CASE("water is drawn blue") {
    const TileAppearance look = tileAppearance(Tile{-1, false});
    REQUIRE(look.fill.blue == 120);
    REQUIRE_FALSE(look.deck);
}

TEST_CASE("a bridge is water with a ground deck") {
    const TileAppearance look = tileAppearance(Tile{0, true});
    REQUIRE(look.fill.blue == 120);
    REQUIRE(look.deck);
    REQUIRE(look.deckFill.red == 24);
    REQUIRE(look.deckFill.green == 36);
}

TEST_CASE("unit color follows the navigation layer") {
    REQUIRE(unitColor(NavLayer::Ground).red == 220);
    REQUIRE(unitColor(NavLayer::Air).blue == 220);
    REQUIRE(unitColor(NavLayer::Underwater).blue == 255);
}
