#pragma once

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
