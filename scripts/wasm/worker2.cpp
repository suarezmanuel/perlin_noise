#include <cstdio> 
#include <random> 
#include <functional> 
#include <iostream> 
#include <fstream> 
#include <cmath> 
#include <vector> // Added for std::vector
#include <stdint.h>
#include <filesystem>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../include/stb_image_write.h"

// https://stackoverflow.com/questions/13772567/how-to-get-the-cpu-cycle-count-in-x86-64-from-c
//  Windows
#ifdef _WIN32
    #include <intrin.h>
    uint64_t rdtsc(){
        // Serialize with CPUID
        int cpuInfo[4];
        __cpuid(cpuInfo, 0);
        return __rdtsc();
    }
#else
    uint64_t rdtsc(){
        unsigned int lo, hi;
        // Serialize with CPUID
        __asm__ __volatile__ (
            "cpuid\n"        // Serialize
            "rdtsc"
            : "=a" (lo), "=d" (hi)
            : "a" (0)
            : "ebx", "ecx");
        return ((uint64_t)hi << 32) | lo;
    }
#endif

typedef float f32;
typedef uint32_t u32;

template<typename T> 
class Vec2 
{ 
public: 
    Vec2() : x(T(0)), y(T(0)) {} 
    Vec2(T xx, T yy) : x(xx), y(yy) {} 
    Vec2 operator * (const T &r) const { return Vec2(x * r, y * r); } 
    T x, y; 
    f32 length2 () { return x*x + y*y; }
    f32 length () { return sqrtf(length2()); }
}; 

template<typename T> 
class Vec3 
{ 
public: 
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {} 
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {} 
    Vec3 operator * (const T &r) const { return Vec3(x * r, y * r, z * r); } 
    T x, y, z; 
    f32 length3 () { return x*x + y*y + z*z; }
    f32 length () { return sqrtf(length3()); }
}; 

typedef Vec3<f32> Vec3f; 

inline
f32 lerp(const f32 &t, const f32 &a, const f32 &b) { return a + t * (b-a); }

inline
f32 quintic(const f32 &t) { return t * t * t * (t * (t * 6 - 15) + 10); }

// calculates dot product with vector of the regular tetrahedron
f32 gradient (uint8_t hash, f32 x, f32 y, f32 z) {

    int h = hash & 15;
    f32 u = h < 8 ? x : y;
    f32 v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    // if h even, if h is 2 or 3 mod 4
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v); 
}

static const unsigned kMaxTableSize = 256; 
static const unsigned kMaxTableSizeMask = kMaxTableSize - 1;
// only works for 512 for now
unsigned int permutation[512] = 
{
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

f32 perlinNoise(const Vec3f &p) 
{
    int X = ((int)std::floor(p.x)) & kMaxTableSizeMask; // find cube of point
    int Y = ((int)std::floor(p.y)) & kMaxTableSizeMask;
    int Z = ((int)std::floor(p.z)) & kMaxTableSizeMask;

    f32 x = p.x - std::floor(p.x);  // find relative x,y,z in cube
    f32 y = p.y - std::floor(p.y);
    f32 z = p.z - std::floor(p.z);

    f32 u = quintic(x);
    f32 v = quintic(y);
    f32 w = quintic(z);

    int A  = permutation[X]   + Y;
    int AA = permutation[A]   + Z;
    int AB = permutation[A+1] + Z;
    int B  = permutation[X+1] + Y;
    int BA = permutation[B]   + Z;
    int BB = permutation[B+1] + Z;

    f32 a = gradient(permutation[AA], x,   y,   z);
    f32 b = gradient(permutation[BA], x-1, y,   z);
    f32 c = gradient(permutation[AB], x,   y-1, z);
    f32 d = gradient(permutation[BB], x-1, y-1, z);

    f32 e = gradient(permutation[AA+1], x,   y,   z-1);
    f32 f = gradient(permutation[BA+1], x-1, y,   z-1);
    f32 g = gradient(permutation[AB+1], x,   y-1, z-1);
    f32 h = gradient(permutation[BB+1], x-1, y-1, z-1);

    f32 l1 = lerp(u, a, b);
    f32 l2 = lerp(u, c, d);
    f32 l3 = lerp(u, e, f);
    f32 l4 = lerp(u, g, h);

    f32 l5 = lerp(v, l1, l2);
    f32 l6 = lerp(v, l3, l4);

    return lerp(w, l5, l6);
}

void perlinNoise_8x(f32 x, f32 y, f32 z, f32 *data) 
{
    for (u32 i=0; i<8; i++) {
        data[i] = perlinNoise(Vec3f(x+i,y,z));
    }
}


int main(int argc, char **argv) 
{ 
    unsigned imageWidth = 512; 
    unsigned imageHeight = 512; 
    // Using std::vector instead of raw pointer
    f32 noiseMap [imageWidth * imageHeight]; 

    // generate value noise
    f32 frequency = 1/32.; 
    std::vector<int> cycleCounts(imageHeight*imageWidth, 0);

    // for (unsigned j = 0; j < imageHeight; ++j) { 
    //     for (unsigned i = 0; i < imageWidth; ++i) { 
    //         int start = rdtsc();
    //         // generate a f32 in the range [0:1]
    //         noiseMap[j * imageWidth + i] = perlinNoise(Vec3f(i, 0, j) * frequency);
    //         cycleCounts[j * imageWidth + i] = (static_cast<int>(rdtsc()-start));
    //     } 
    // } 

    for (unsigned j = 0; j < imageHeight; ++j) { 
        for (unsigned i = 0; i < imageWidth; i+=8) { 
            int start = rdtsc();
            // generate a f32 in the range [0:1]
            // noiseMap[j * imageWidth + i] = perlinNoise(Vec3f(i, 0, j) * frequency);
            perlinNoise_8x(i* frequency, 0* frequency, j* frequency, noiseMap+(j * imageWidth + i));
            cycleCounts[j * imageWidth + i] = (static_cast<int>(rdtsc()-start));
        } 
    } 

    long long sumCycles = 0;
    for (auto cycles : cycleCounts) {
        sumCycles += cycles;
    }

    std::cout << "cycle count: " << sumCycles << " cycles" << std::endl;
    std::cout << "average cycle count per call: " << sumCycles / (imageWidth * imageHeight) << " cycles" << std::endl;

    // Save cycle counts to a file for plotting
    std::ofstream cycleFile("./scripts/plot/cycle_counts.dat");
    if (!cycleFile) {
        std::cerr << "Failed to open cycle_counts.dat for writing.\n";
        return 1;
    }

    // Write data in the format: x y cycles
    for(unsigned j = 0; j < imageHeight; ++j){
        for(unsigned i = 0; i < imageWidth; ++i){
            int index = j * imageWidth + i;
            cycleFile << j * imageWidth + i << " " << cycleCounts[index] << "\n";
        }
    }
    cycleFile.close();

// Prepare image buffer (RGB)
    std::vector<unsigned char> imageBuffer(imageWidth * imageHeight * 3);

    for (unsigned k = 0; k < imageWidth * imageHeight; ++k) { 
        // Normalize noiseMap[k] from [-1,1] to [0,255]
        unsigned char n = static_cast<unsigned char>((noiseMap[k] + 1.0f) * 127.5f); 
        imageBuffer[3 * k + 0] = n; // Red channel
        imageBuffer[3 * k + 1] = n; // Green channel
        imageBuffer[3 * k + 2] = n; // Blue channel
    }

    std::string outputFilename = "./resources/noise/noise.png";
    // overwrite if already exists
    if (stbi_write_png(outputFilename.c_str(), imageWidth, imageHeight, 3, imageBuffer.data(), imageWidth * 3)) {
        std::cout << "Successfully wrote " << outputFilename << "\n";
    } else {
        std::cerr << "Failed to write " << outputFilename << "\n";
        return 1;
    }
    return 0;
}
