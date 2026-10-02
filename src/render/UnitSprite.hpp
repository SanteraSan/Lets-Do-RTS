#pragma once

#include <cstddef>
#include <utility>
#include <vector>

struct SpriteImage {
    int width = 0;
    int height = 0;
    std::vector<unsigned char> rgba;
};

inline SpriteImage makeUnitSprite() {
    constexpr int kSize = 32;
    SpriteImage image;
    image.width = kSize;
    image.height = kSize;
    image.rgba.assign(static_cast<std::size_t>(kSize * kSize * 4), 0);

    const float center = (static_cast<float>(kSize) - 1.0f) * 0.5f;
    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x < kSize; ++x) {
            const float dx = (static_cast<float>(x) - center) / 10.0f;
            const float bodyDy = (static_cast<float>(y) - (center + 2.0f)) / 12.0f;
            const float headDy = (static_cast<float>(y) - (center - 8.0f)) / 5.0f;
            const float headDx = (static_cast<float>(x) - center) / 5.0f;
            const bool body = dx * dx + bodyDy * bodyDy <= 1.0f;
            const bool head = headDx * headDx + headDy * headDy <= 1.0f;
            if (!body && !head) {
                continue;
            }
            const std::size_t index = static_cast<std::size_t>((y * kSize + x) * 4);
            image.rgba[index] = 255;
            image.rgba[index + 1] = 255;
            image.rgba[index + 2] = 255;
            image.rgba[index + 3] = 255;
        }
    }
    return image;
}

inline void trimSprite(SpriteImage& image) {
    if (image.width <= 0 || image.height <= 0) {
        return;
    }
    const std::size_t need =
        static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height) * 4;
    if (image.rgba.size() < need) {
        return;
    }
    int minX = image.width;
    int minY = image.height;
    int maxX = -1;
    int maxY = -1;
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            const std::size_t alpha =
                (static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width) + static_cast<std::size_t>(x)) * 4 + 3;
            if (image.rgba[alpha] == 0) {
                continue;
            }
            if (x < minX) {
                minX = x;
            }
            if (y < minY) {
                minY = y;
            }
            if (x > maxX) {
                maxX = x;
            }
            if (y > maxY) {
                maxY = y;
            }
        }
    }
    if (maxX < 0) {
        return;
    }
    const int width = maxX - minX + 1;
    const int height = maxY - minY + 1;
    std::vector<unsigned char> trimmed(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const std::size_t source =
                (static_cast<std::size_t>(y + minY) * static_cast<std::size_t>(image.width) + static_cast<std::size_t>(x + minX)) * 4;
            const std::size_t dest = (static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)) * 4;
            trimmed[dest] = image.rgba[source];
            trimmed[dest + 1] = image.rgba[source + 1];
            trimmed[dest + 2] = image.rgba[source + 2];
            trimmed[dest + 3] = image.rgba[source + 3];
        }
    }
    image.width = width;
    image.height = height;
    image.rgba = std::move(trimmed);
}
