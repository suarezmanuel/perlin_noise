#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <emscripten/bind.h>
#include <emscripten/val.h>

struct Vec2 {
    double x;
    double y;
};

const Vec2& get_random_vec(double x, double y, double seed, const std::vector<Vec2>& vectors) {
    uint32_t h = static_cast<uint32_t>(seed + x*328413u + y*191117u);
    return vectors[h % vectors.size()];
}

// dot product of two Vec2 vectors
double dot_product(const Vec2& v, const Vec2& u) {
    return v.x * u.x + v.y * u.y;
}

// cubic interpolation (smooth step)
double cubic_interpolate(double a, double b, double w) {
    return (b - a) * (3.0 - w * 2.0) * w * w + a;
}

// perlin noise function
double perlin(double x, double y, double seed, const std::vector<Vec2>& vectors) {

    int32_t x0 = static_cast<int32_t>(std::floor(x));
    int32_t x1 = x0 + 1;
    int32_t y0 = static_cast<int32_t>(std::floor(y));
    int32_t y1 = y0 + 1;

    Vec2 v00 = { x - x0, y - y0 };
    Vec2 v10 = { x - x1, y - y0 };
    Vec2 v01 = { x - x0, y - y1 };
    Vec2 v11 = { x - x1, y - y1 };

    double s = dot_product(v00, get_random_vec(x0, y0, seed, vectors));
    double t = dot_product(v10, get_random_vec(x1, y0, seed, vectors));
    double u = dot_product(v01, get_random_vec(x0, y1, seed, vectors));
    double v = dot_product(v11, get_random_vec(x1, y1, seed, vectors));

    double sx = x - x0;
    double sy = y - y0;

    double a = cubic_interpolate(s, t, sx);
    double b = cubic_interpolate(u, v, sx);

    return cubic_interpolate(a, b, sy);
}

void dispatchWorker(int startX, int startY, int width, int height, int layer_count, int grid_size, double seed, std::vector<Vec2>& vectors, uint8_t*& noise) {

    for (int32_t y = 0; y < height; y++) {
        for (int32_t x = startX; x < width; x++) {

            double amp = 1.0;
            double freq = 1.0;
            double val = 0.0;

            for (int32_t i = 0; i < layer_count; i++) {
                val += perlin(x * freq / grid_size, y * freq / grid_size, seed, vectors) * amp;
                freq *= 2.0;
                amp /= 2.0;
            }

            val *= 1.2;
            val = std::min(1.0, std::max(-1.0, val));
            int index = y * width + x;
            noise[index] = static_cast<uint8_t>(std::floor((val + 1.0) * 127.5));
        }
    }
}

uintptr_t generateNoise(int width, int height, int grid_size, int layer_count, double seed, emscripten::val vectorsGivenVal) {

    std::vector<Vec2> vectors;
    const int length = vectorsGivenVal["length"].as<int>();
    vectors.reserve(length);

    for (int i = 0; i < length; i++) {
        emscripten::val vecVal = vectorsGivenVal[i];
        Vec2 vec;
        vec.x = vecVal[0].as<double>();
        vec.y = vecVal[1].as<double>();
        vectors.push_back(vec);
    }

    uint8_t* noise = (uint8_t*)malloc(width * height);
    dispatchWorker(0, 0, width, height, layer_count, grid_size, seed, vectors, noise);
    return reinterpret_cast<uintptr_t>(noise);
}

void freeNoise(uintptr_t noisePtr) {
    uint8_t* noise = reinterpret_cast<uint8_t*>(noisePtr);
    free(noise);
}

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("generateNoise", &generateNoise);
    emscripten::function("freeNoise", &freeNoise);
}