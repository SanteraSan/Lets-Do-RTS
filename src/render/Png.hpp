#pragma once

#include "render/UnitSprite.hpp"

#include <cstddef>

bool loadPng(const unsigned char* data, std::size_t size, SpriteImage& image);
bool loadPngFile(const char* path, SpriteImage& image);
