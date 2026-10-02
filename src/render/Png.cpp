#include "render/Png.hpp"

#include <fstream>
#include <vector>

#include <zlib.h>

namespace {

unsigned readBe32(const unsigned char* bytes) {
    return (static_cast<unsigned>(bytes[0]) << 24) | (static_cast<unsigned>(bytes[1]) << 16) |
        (static_cast<unsigned>(bytes[2]) << 8) | static_cast<unsigned>(bytes[3]);
}

bool sameType(const unsigned char* bytes, char a, char b, char c, char d) {
    return bytes[0] == static_cast<unsigned char>(a) && bytes[1] == static_cast<unsigned char>(b) &&
        bytes[2] == static_cast<unsigned char>(c) && bytes[3] == static_cast<unsigned char>(d);
}

int paeth(int left, int up, int upLeft) {
    const int estimate = left + up - upLeft;
    const int deltaLeft = estimate > left ? estimate - left : left - estimate;
    const int deltaUp = estimate > up ? estimate - up : up - estimate;
    const int deltaUpLeft = estimate > upLeft ? estimate - upLeft : upLeft - estimate;
    if (deltaLeft <= deltaUp && deltaLeft <= deltaUpLeft) {
        return left;
    }
    if (deltaUp <= deltaUpLeft) {
        return up;
    }
    return upLeft;
}

bool inflateIdat(const std::vector<unsigned char>& idat, std::vector<unsigned char>& raw) {
    z_stream stream{};
    stream.next_in = const_cast<unsigned char*>(idat.data());
    stream.avail_in = static_cast<unsigned>(idat.size());
    if (inflateInit(&stream) != Z_OK) {
        return false;
    }
    stream.next_out = raw.data();
    stream.avail_out = static_cast<unsigned>(raw.size());
    const int result = inflate(&stream, Z_FINISH);
    const unsigned produced = static_cast<unsigned>(stream.total_out);
    inflateEnd(&stream);
    return result == Z_STREAM_END && produced == raw.size();
}

} // namespace

bool loadPng(const unsigned char* data, std::size_t size, SpriteImage& image) {
    image = SpriteImage{};
    const unsigned char signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    if (data == nullptr || size < 8) {
        return false;
    }
    for (int i = 0; i < 8; ++i) {
        if (data[i] != signature[i]) {
            return false;
        }
    }

    int width = 0;
    int height = 0;
    int bitDepth = 0;
    int colorType = 0;
    int interlace = 0;
    bool haveHeader = false;
    std::vector<unsigned char> idat;
    std::size_t pos = 8;
    while (pos + 8 <= size) {
        const unsigned length = readBe32(data + pos);
        if (pos + 12ull + length > size) {
            return false;
        }
        const unsigned char* type = data + pos + 4;
        const unsigned char* chunk = data + pos + 8;
        if (sameType(type, 'I', 'H', 'D', 'R')) {
            if (length < 13) {
                return false;
            }
            width = static_cast<int>(readBe32(chunk));
            height = static_cast<int>(readBe32(chunk + 4));
            bitDepth = chunk[8];
            colorType = chunk[9];
            interlace = chunk[12];
            if (chunk[10] != 0 || chunk[11] != 0) {
                return false;
            }
            haveHeader = true;
        } else if (sameType(type, 'I', 'D', 'A', 'T')) {
            idat.insert(idat.end(), chunk, chunk + length);
        } else if (sameType(type, 'I', 'E', 'N', 'D')) {
            break;
        }
        pos += 12ull + length;
    }

    if (!haveHeader || idat.empty() || bitDepth != 8 || interlace != 0) {
        return false;
    }
    if (colorType != 2 && colorType != 6) {
        return false;
    }
    if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
        return false;
    }

    const int channels = colorType == 6 ? 4 : 3;
    const std::size_t rowBytes = static_cast<std::size_t>(width) * static_cast<std::size_t>(channels);
    const std::size_t expected = static_cast<std::size_t>(height) * (rowBytes + 1);
    if (expected > static_cast<std::size_t>(~0u)) {
        return false;
    }
    std::vector<unsigned char> raw(expected);
    if (!inflateIdat(idat, raw)) {
        return false;
    }

    std::vector<unsigned char> pixels(static_cast<std::size_t>(height) * rowBytes);
    std::size_t rawPos = 0;
    for (int y = 0; y < height; ++y) {
        const int filter = raw[rawPos++];
        if (filter > 4) {
            return false;
        }
        unsigned char* row = pixels.data() + static_cast<std::size_t>(y) * rowBytes;
        const unsigned char* previous = y > 0 ? row - rowBytes : nullptr;
        for (std::size_t i = 0; i < rowBytes; ++i) {
            const int sample = raw[rawPos++];
            const int left = i >= static_cast<std::size_t>(channels) ? row[i - static_cast<std::size_t>(channels)] : 0;
            const int up = previous == nullptr ? 0 : previous[i];
            const int upLeft = previous == nullptr || i < static_cast<std::size_t>(channels)
                ? 0
                : previous[i - static_cast<std::size_t>(channels)];
            int value = sample;
            if (filter == 1) {
                value += left;
            } else if (filter == 2) {
                value += up;
            } else if (filter == 3) {
                value += (left + up) / 2;
            } else if (filter == 4) {
                value += paeth(left, up, upLeft);
            }
            row[i] = static_cast<unsigned char>(value & 255);
        }
    }

    image.width = width;
    image.height = height;
    image.rgba.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4, 255);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const std::size_t source =
                static_cast<std::size_t>(y) * rowBytes + static_cast<std::size_t>(x) * static_cast<std::size_t>(channels);
            const std::size_t dest = (static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)) * 4;
            image.rgba[dest] = pixels[source];
            image.rgba[dest + 1] = pixels[source + 1];
            image.rgba[dest + 2] = pixels[source + 2];
            if (channels == 4) {
                image.rgba[dest + 3] = pixels[source + 3];
            }
        }
    }
    return true;
}

bool loadPngFile(const char* path, SpriteImage& image) {
    image = SpriteImage{};
    if (path == nullptr) {
        return false;
    }
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    std::vector<unsigned char> bytes(
        (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return loadPng(bytes.data(), bytes.size(), image);
}
