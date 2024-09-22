#include <vector>
#include <cmath>
#include <cstdint>
#include <emscripten/bind.h>

using namespace emscripten;

std::vector<std::vector<double>&> vectors;

std::vector<uint8_t> main( int startX, int endX, int width, 
           int height, int grid_size, int layer_count,
           double seed, const std::vector<int>& buffer,
           val vectorsGiven ) {

    vectors = vectorsGiven;
    int32_t vectorsLength = vectorsGiven["length"].as<int32_t>();
    for (int32_t i = 0; i < vectorsLength; ++i) {
        val vec = vectorsGiven[i];
        double x = vec[0].as<double>();
        double y = vec[1].as<double>();
        vectors.push_back({ x, y });
    }

    // vector of size width*height filled with zeroes
    std::vector<uint8_t> perlinMatrix (width*height, 0);

    for (int32_t x=0; x < height; x++) {
        for (int32_t y=startX; y < endX; y++) {

            double amp = 1;
            double freq = 1;
            double val = 0;
            
            for (int32_t i=0; i < layer_count; i++) {
                val += perlin(x*freq/grid_size ,y*freq/grid_size, seed) * amp;
                freq *= 2;
                amp /= 2;
            }

            val *= 1.2;
            val = std::min(1, std::max(-1, val));
            // convert range [-1, 1] to [0, 255], 127.5 = 255/2
            perlinMatrix[x * width + y] = std::floor((val+1) * 127.5);
        }
    }

    // postMessage({ status: 'done' });
}

float perlin (double x, double y, double seed) {

    int32_t x0 = static_cast<int32_t>(std::floor(x));
    int32_t x1 = static_cast<int32_t>(std::ceil(x));
    int32_t y0 = static_cast<int32_t>(std::floor(y));
    int32_t y1 = static_cast<int32_t>(std::ceil(y));

    std::vector<double> v00 = {x0-x, y0-y};
    std::vector<double> v10 = {x1-x, y0-y};
    std::vector<double> v01 = {x0-x, y1-y};
    std::vector<double> v11 = {x1-x, y1-y};

    int32_t t1 = cubic_interpolate(
                    dot_product(v00, get_random_vec(x0,y0,seed)),
                    dot_product(v10, get_random_vec(x1,y0,seed)),
                    x-x0
                );

    int32_t t2 = cubic_interpolate(
                    dot_product(v01, get_random_vec(x0,y1,seed)), 
                    dot_product(v11, get_random_vec(x1,y1,seed)), 
                    x-x0
                );

    return cubic_interpolate(t1, t2, y-y0);
}

// we want the same points to get the same randoms
// other than that, its completely random
std::vector<double> get_random_vec (double x, double y, double seed) {
    const int32_t hash = static_cast<int32_t>(std::floor(seed + x*3284157443 + seed + y*1911520717));
    return vectors[hash % vectors.size()];
}

double seeded_random (double seed) {
    double x = sin(seed) * 10000;
    return x - std::floor(x);
}

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