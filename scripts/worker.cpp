#include <vector>
#include <cmath>
#include <cstdint>
#include <emscripten/bind.h>
#include <algorithm>

// we want the same points to get the same randoms
// other than that, its completely random
std::vector<double> get_random_vec (double x, double y, double seed, std::vector<std::vector<double>> vectors) {
    const int32_t hash = static_cast<int32_t>(std::floor(seed + x*3284157443 + seed + y*1911520717));
    // vectors = vectorsGiven
    return vectors[hash % vectors.size()];
}

// double seeded_random (double seed) {
//     double x = sin(seed) * 10000;
//     return x - std::floor(x);
// }

// imagine being on the unit circle
std::vector<double> get_unit_vector (double theta) {
    return { std::cos(theta), std::sin(theta) };
}

// dot product = inner product in R^2
double dot_product (std::vector<double> v, std::vector<double> u) {
    return v[0]*u[0] + v[1]*u[1];
}

// smooth step
double cubic_interpolate (double a, double b, double w) {
    return (b - a) * (3.0 - w * 2.0) * w * w + a;
}

float perlin (double x, double y, double seed, std::vector<std::vector<double>> vectors) {

    int32_t x0 = static_cast<int32_t>(std::floor(x));
    int32_t x1 = static_cast<int32_t>(std::ceil(x));
    int32_t y0 = static_cast<int32_t>(std::floor(y));
    int32_t y1 = static_cast<int32_t>(std::ceil(y));

    std::vector<double> v00 = {x0-x, y0-y};
    std::vector<double> v10 = {x1-x, y0-y};
    std::vector<double> v01 = {x0-x, y1-y};
    std::vector<double> v11 = {x1-x, y1-y};

    int32_t t1 = cubic_interpolate(
                    dot_product(v00, get_random_vec(x0,y0,seed, vectors)),
                    dot_product(v10, get_random_vec(x1,y0,seed, vectors)),
                    x-x0
                );

    int32_t t2 = cubic_interpolate(
                    dot_product(v01, get_random_vec(x0,y1,seed, vectors)), 
                    dot_product(v11, get_random_vec(x1,y1,seed, vectors)), 
                    x-x0
                );

    return cubic_interpolate(t1, t2, y-y0);
}

void generatePerlinMatrix  (    int startX, int endX, int width, 
                                                int height, int grid_size, int layer_count,
                                                double seed, emscripten::val bufferVal,
                                                const std::vector<std::vector<double>>& vectors ) {

    auto bufferArray = emscripten::val::global("Uint8Array").new_(bufferVal);
    
    for (int32_t x=0; x < height; x++) {
        for (int32_t y=startX; y < endX; y++) {

            double amp = 1;
            double freq = 1;
            double val = 0;
            
            for (int32_t i=0; i < layer_count; i++) {
                val += perlin(x*freq/grid_size ,y*freq/grid_size, seed, vectors) * amp;
                freq *= 2;
                amp /= 2;
            }

            val *= 1.2;
            val = std::min(1.0, std::max(-1.0, val));
            // convert range [-1, 1] to [0, 255], 127.5 = 255/2
            bufferArray.set(x * width + y, std::floor((val+1) * 127.5));
        }
    }
}

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("generatePerlinMatrix", &generatePerlinMatrix);
    emscripten::register_vector<double>("DoubleVector");
    emscripten::register_vector<std::vector<double>>("VectorOfDoubleVector");
}