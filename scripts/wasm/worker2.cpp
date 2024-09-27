#include <cstdio> 
#include <random> 
#include <functional> 
#include <iostream> 
#include <fstream> 
#include <cmath> 
#include <vector> // Added for std::vector
#include <stdint.h>
#include <filesystem>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
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

typedef float f32;
typedef uint32_t u32;
typedef int32_t s32;
static const unsigned kMaxTableSize = 256; 
static const unsigned kMaxTableSizeMask = kMaxTableSize - 1;

// different representations of the same thing
union f32_4x
{
    __m128 sse;
    f32 E[4];
};

f32_4x F32_4X(f32 A, f32 B, f32 C, f32 D) {
    // set backwards
    f32_4x result = {{ _mm_set_ps(D,C,B,A) }};
    return result;
}

f32_4x F32_4X(f32 A) {
    // set backwards
    f32_4x result = {{ _mm_set_ps1(A) }};
    return result;
}

inline f32_4x
operator+ (f32_4x A, f32_4x B)
{
    f32_4x result = {{ _mm_add_ps(A.sse, B.sse) }};
    return result;
}

inline f32_4x
operator- (f32_4x A, f32_4x B)
{
    f32_4x result = {{ _mm_sub_ps(A.sse, B.sse) }};
    return result;
}

inline f32_4x
operator* (f32_4x A, f32_4x B)
{
    f32_4x result = {{ _mm_mul_ps(A.sse, B.sse) }};
    return result;
}

inline f32_4x
operator& (f32_4x A, f32_4x B)
{
    f32_4x result = {{ _mm_and_ps(A.sse, B.sse) }};
    return result;
}

inline f32_4x
operator| (f32_4x A, f32_4x B)
{
    f32_4x result = {{ _mm_or_ps(A.sse, B.sse) }};
    return result;
}

inline f32_4x
operator==(f32_4x A, f32_4x B)
{
  f32_4x result = {{ _mm_cmpeq_ps(A.sse, B.sse) }};
  return result;
}

inline f32_4x
operator>(f32_4x A, f32_4x B)
{
  f32_4x result = {{ _mm_cmpgt_ps(A.sse, B.sse) }};
  return result;
}

inline f32_4x
operator<(f32_4x A, f32_4x B)
{
  f32_4x result = {{ _mm_cmplt_ps(A.sse, B.sse) }};
  return result;
}


// u32_4x

// union u32_4x
// {
//     __m128i sse;
//     u32 E[4];
// };

// u32_4x U32_4X(u32 A, u32 B, u32 C, u32 D) {
//     // set backwards
//     u32_4x result = {{ _mm_set_epi32(s32(D),s32(C),s32(B),s32(A)) }};
//     return result;
// }

// u32_4x U32_4X(u32 A) {
//     // set backwards
//     u32_4x result = U32_4X(A, A, A, A);
//     return result;
// }

// inline u32_4x
// operator+(u32_4x A, u32_4x B)
// {
//   u32_4x Result = {{ _mm_add_epi32(A.sse, B.sse) }};
//   return Result;
// }

// inline u32_4x
// operator-(u32_4x A, u32_4x B)
// {
//   u32_4x Result = {{ _mm_sub_epi32(A.sse, B.sse) }};
//   return Result;
// }

// inline u32_4x
// operator*(u32_4x A, u32_4x B)
// {
//   u32_4x Result = {{ _mm_mul_epi32(A.sse, B.sse) }};
//   return Result;
// }

// NOTE(Jesse): Apparently this one doesn't exist?!
// #if 0
// inline u32_4x
// operator/(u32_4x A, u32_4x B)
// {
//   u32_4x Result = {{ _mm_div_epi32(A.sse, B.sse) }};
//   return Result;
// }
// #endif

// inline u32_4x
// operator==(u32_4x A, u32_4x B)
// {
//   u32_4x Result = {{ _mm_cmpeq_epi32(A.sse, B.sse) }};
//   return Result;
// }

// inline u32_4x
// operator>(u32_4x A, u32_4x B)
// {
//   u32_4x Result = {{ _mm_cmpgt_epi32(A.sse, B.sse) }};
//   return Result;
// }

// inline u32_4x
// operator<(u32_4x A, u32_4x B)
// {
//   u32_4x Result = {{ _mm_cmplt_epi32(A.sse, B.sse) }};
//   return Result;
// }

// inline u32_4x
// operator|(u32_4x A, u32_4x B)
// {
//   u32_4x Result = {{ _mm_or_si128(A.sse, B.sse) }};
//   return Result;
// }

// inline u32_4x
// operator&(u32_4x A, u32_4x B)
// {
//   u32_4x Result = {{ _mm_and_si128(A.sse, B.sse) }};
//   return Result;
// }

// inline u32_4x
// operator&(u32_4x A, s32 B)
// {
//   u32_4x B4 = U32_4X(u32(B));
//   u32_4x Result = A & B4;
//   return Result;
// }

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

typedef Vec2<f32> Vec2f; 
typedef Vec3<f32> Vec3f; 

inline
f32 lerp(const f32 &t, const f32 &a, const f32 &b) { return a + t * (b-a); }

f32_4x lerp_4x(const f32_4x &t, const f32_4x &a, const f32_4x &b) { return a + t * (b-a); }

inline
f32 quintic(const f32 &t) { return t * t * t * (t * (t * 6 - 15) + 10); }

inline
f32_4x _select(f32_4x mask, f32_4x A, f32_4x B) {
    f32_4x result;
    result.sse = _mm_blendv_ps(B.sse, A.sse, mask.sse);;
    return result;
}

// calculates dot product with vector of the regular tetrahedron
f32 gradient (uint8_t hash, f32 x, f32 y, f32 z) {

    auto h = hash & 15;

    auto uSel = h < 8;
    auto vSel = h < 4;
    auto xSel = (h == 12 | h == 14);

    auto u  = uSel ? x : y;
    auto xz = xSel ? x : z;
    auto v  = vSel ? y : xz;

    auto uFlip = (h & 1) == 1;
    auto vFlip = (h & 2) == 2;

    auto R0 = uFlip ? -1*u : u;
    auto R1 = vFlip ? -1*v : v;
    return R0 + R1;
}

f32_4x gradient_4x (f32_4x hash, f32_4x x, f32_4x y, f32_4x z) {
    // pack scalars
    auto _15 = F32_4X(15);
    auto _14 = F32_4X(14);
    auto _12 = F32_4X(12);
    auto _8  = F32_4X(8);
    auto _4  = F32_4X(4);
    auto _2  = F32_4X(2);
    auto _1  = F32_4X(1);
    auto _n1 = F32_4X(-1);

    auto h = hash & _15;

    auto uSel = h < _8;
    auto vSel = h < _4;
    auto xSel = (h == _12 | h == _14);

    // selects either x or y based on interpolation via uSel
    auto u  = _select(uSel, x, y);
    auto xz = _select(xSel, x, z);
    auto v  = _select(vSel, y, xz);

    auto uFlip = (h & _1) == _1;
    auto vFlip = (h & _2) == _2;

    auto R0 = _select(uFlip, _n1*u, u);
    auto R1 = _select(vFlip, _n1*v, v);
    return R0 + R1;
}

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

    int H0 = permutation[AA];
    int H1 = permutation[BA];
    int H2 = permutation[AB];
    int H3 = permutation[BB];
    int H4 = permutation[AA+1];
    int H5 = permutation[BA+1];
    int H6 = permutation[AB+1];
    int H7 = permutation[BB+1];

    f32 G0 = gradient(H0, x,   y,   z);
    f32 G1 = gradient(H1, x-1, y,   z);
    f32 G2 = gradient(H2, x,   y-1, z);
    f32 G3 = gradient(H3, x-1, y-1, z);

    f32 G4 = gradient(H4, x,   y,   z-1);
    f32 G5 = gradient(H5, x-1, y,   z-1);
    f32 G6 = gradient(H6, x,   y-1, z-1);
    f32 G7 = gradient(H7, x-1, y-1, z-1);

    f32 L1 = lerp(u, G0, G1);
    f32 L2 = lerp(u, G2, G3);
    f32 L3 = lerp(u, G4, G5);
    f32 L4 = lerp(u, G6, G7);

    f32 L5 = lerp(v, L1, L2);
    f32 L6 = lerp(v, L3, L4);

    return lerp(w, L5, L6);
}

f32 perlinNoiseSIMD(const Vec3f &p) 
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

    int H0 = permutation[AA];
    int H1 = permutation[BA];
    int H2 = permutation[AB];
    int H3 = permutation[BB];
    int H4 = permutation[AA+1];
    int H5 = permutation[BA+1];
    int H6 = permutation[AB+1];
    int H7 = permutation[BB+1];

    f32_4x x_x_x_x     = F32_4X(x, x, x, x);
    f32_4x nx_nx_nx_nx = F32_4X(x-1, x-1, x-1, x-1);
    f32_4x y_ny_y_ny   = F32_4X(y, y-1, y, y-1);
    f32_4x z_z_nz_nz   = F32_4X(z, z, z-1, z-1);

    f32_4x H0246 = F32_4X(H0, H2, H4, H6);
    f32_4x H1357 = F32_4X(H1, H3, H5, H7);

    f32_4x A4 = gradient_4x(H0246, x_x_x_x, y_ny_y_ny, z_z_nz_nz);
    f32_4x B4 = gradient_4x(H1357, nx_nx_nx_nx, y_ny_y_ny, z_z_nz_nz);

    f32_4x u_4x = F32_4X(u);

    f32_4x L0123 = lerp_4x(u_4x, A4, B4);

    f32 L5 = lerp(v, L0123.E[0], L0123.E[1]);
    f32 L6 = lerp(v, L0123.E[2], L0123.E[3]);

    return lerp(w, L5, L6);
}

void perlinNoise_8x(f32 x, f32 y, f32 z, f32 *data, f32 frequency) 
{
    for (u32 i=0; i<8; i++) {
        data[i] = perlinNoiseSIMD(Vec3f(x+i,y,z)*frequency);
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
    std::vector<long long> cycleCounts(imageHeight*imageWidth, 0);

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
            perlinNoise_8x(i, 0.0, j, noiseMap+(j * imageWidth + i), frequency);
            cycleCounts[(j * imageWidth + i)/8] = (static_cast<int>(rdtsc()-start));
        } 
    } 

    long long sumCycles = 0;
    long long min = cycleCounts[0];
    long long max = cycleCounts[0];
    for (auto cycles : cycleCounts) {
        sumCycles += cycles;
        min = std::min(min, cycles);
        max = std::max(max, cycles);
    }

    std::cout << "cycle count: " << sumCycles << " cycles" << std::endl;
    std::cout << "min cycle count in a call: " << min << " cycles" << std::endl;
    std::cout << "max cycle count in a call: " << max << " cycles" << std::endl;
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
