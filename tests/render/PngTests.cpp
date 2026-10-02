#include "render/Png.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("png loader reads a 2 by 2 rgba image") {
    const unsigned char png[] = {137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 2, 0, 0, 0,
        2, 8, 6, 0, 0, 0, 114, 182, 13, 36, 0, 0, 0, 20, 73, 68, 65, 84, 120, 156, 99, 248, 207, 192, 0, 68, 32, 240,
        31, 196, 248, 223, 0, 0, 45, 101, 5, 124, 67, 7, 31, 221, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130};
    SpriteImage image;
    REQUIRE(loadPng(png, sizeof(png), image));
    REQUIRE(image.width == 2);
    REQUIRE(image.height == 2);
    REQUIRE(image.rgba[0] == 255);
    REQUIRE(image.rgba[3] == 255);
    REQUIRE(image.rgba[7] == 0);
    REQUIRE(image.rgba[9] == 255);
    REQUIRE(image.rgba[14] == 255);
    REQUIRE(image.rgba[15] == 128);
}

TEST_CASE("infantry png has a clear corner and an opaque chest") {
    const std::string path = std::string(RTS_SOURCE_DIR) + "/assets/units/infantry.png";
    SpriteImage image;
    REQUIRE(loadPngFile(path.c_str(), image));
    REQUIRE(image.width == 1254);
    REQUIRE(image.height == 1254);
    REQUIRE(image.rgba[3] == 0);
    const std::size_t chest = (static_cast<std::size_t>(600) * 1254 + 600) * 4;
    REQUIRE(image.rgba[chest] == 173);
    REQUIRE(image.rgba[chest + 1] == 171);
    REQUIRE(image.rgba[chest + 2] == 169);
    REQUIRE(image.rgba[chest + 3] == 255);
}
