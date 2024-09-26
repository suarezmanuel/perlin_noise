#include <cstdio> 
#include <random> 
#include <functional> 
#include <iostream> 
#include <fstream> 
#include <cmath> 
#include <vector> // Added for std::vector
#include <stdint.h>

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


template<typename T> 
class Vec2 
{ 
public: 
    Vec2() : x(T(0)), y(T(0)) {} 
    Vec2(T xx, T yy) : x(xx), y(yy) {} 
    Vec2 operator * (const T &r) const { return Vec2(x * r, y * r); } 
    T x, y; 
    float length2 () { return x*x + y*y; }
    float length () { return sqrtf(length2()); }
}; 

template<typename T> 
class Vec3 
{ 
public: 
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {} 
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {} 
    Vec3 operator * (const T &r) const { return Vec3(x * r, y * r, z * r); } 
    T x, y, z; 
    float length3 () { return x*x + y*y + z*z; }
    float length () { return sqrtf(length3()); }
}; 

typedef Vec3<float> Vec3f; 

template<typename T = float> 
inline T lerp(const T &lo, const T &hi, const T &t) 
{ return lo * (1 - t) + hi * t; } 

inline
float lerp(const float &t, const float &a, const float &b) { return a + t * (b-a); }

inline 
float smoothstep(const float &t) { return t * t * (3 - 2 * t); } 

inline
float quintic(const float &t) { return t * t * t * (t * (t * 6 - 15) + 10); }

inline 
float quinticDeriv(const float &t) { return 30 * t * t * (t * (t - 2) + 1); }
    
inline
float dot (const Vec3f &a, const Vec3f &b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

// calculates dot product with vector of the regular tetrahedron
float gradient (uint8_t hash, float x, float y, float z) {
    // modulo 16
    switch (hash & 15) {
        case  0: return  x + y;  //(1,1,0) 
        case  1: return -x + y;  //(-1,1,0) 
        case  2: return  x - y;  //(1,-1,0) 
        case  3: return -x - y;  //(-1,-1,0) 
        case  4: return  x + z;  //(1,0,1) 
        case  5: return -x + z;  //(-1,0,1) 
        case  6: return  x - z;  //(1,0,-1) 
        case  7: return -x - z;  //(-1,0,-1) 
        case  8: return  y + z;  //(0,1,1), 
        case  9: return -y + z;  //(0,-1,1), 
        case 10: return  y - z;  //(0,1,-1), 
        case 11: return -y - z;  //(0,-1,-1) 
        case 12: ret
    // float evalDeriv(const Vec3f &p) const
    // { 
    //     // modulo kMaxTableSize, its a power of two
    //     int xi0 = ((int)std::floor(p.x)) & kMaxTableSizeMask; 
    //     int yi0 = ((int)std::floor(p.y)) & kMaxTableSizeMask;
    //     int zi0 = ((int)std::floor(p.z)) & kMaxTableSizeMask;

    //     int xi1 = (xi0 + 1) & kMaxTableSizeMask;
    //     int yi1 = (yi0 + 1) & kMaxTableSizeMask; 
    //     int zi1 = (zi0 + 1) & kMaxTableSizeMask; 

    //     // distance from top left to point
    //     float tx = p.x - ((int)std::floor(p.x)); 
    //     float ty = p.y - ((int)std::floor(p.y));
    //     float tz = p.z - ((int)std::floor(p.z));

    //     // remapping of tx and ty using the Smoothstep function 
    //     float u = quintic(tx); 
    //     float v = quintic(ty); 
    //     float w = quintic(tz); 

    //     // vectors from the grid points to p
    //     float x0 = tx, x1 = tx-1;
    //     float y0 = ty, y1 = ty-1;
    //     float z0 = tz, z1 = tz-1;

    //     // seeded random at each corner
    //     float a = gradient(permutationTable[permutationTable[permutationTable[xi0] + yi0] + zi0], x0, y0, z0); 
    //     float b = gradient(permutationTable[permutationTable[permutationTable[xi1] + yi0] + zi0], x1, y0, z0); 
    //     float c = gradient(permutationTable[permutationTable[permutationTable[xi0] + yi1] + zi0], x0, y1, z0); 
    //     float d = gradient(permutationTable[permutationTable[permutationTable[xi1] + yi1] + zi0], x1, y1, z0); 

    //     float e = gradient(permutationTable[permutationTable[permutationTable[xi0] + yi0] + zi1], x0, y0, z1); 
    //     float f = gradient(permutationTable[permutationTable[permutationTable[xi1] + yi0] + zi1], x1, y0, z1); 
    //     float g = gradient(permutationTable[permutationTable[permutationTable[xi0] + yi1] + zi1], x0, y1, z1); 
    //     float h = gradient(permutationTable[permutationTable[permutationTable[xi1] + yi1] + zi1], x1, y1, z1); 

    //     float du = quinticDeriv(tx); 
    //     float dv = quinticDeriv(ty); 
    //     float dw = quinticDeriv(tz); 
 
    //     float k0 = a; 
    //     float k1 = (b - a); 
    //     float k2 = (c - a); 
    //     float k3 = (e - a); 
    //     float k4 = (a + d - b - c); 
    //     float k5 = (a + f - b - e); 
    //     floaturn  y + x;  //(1,1,0) 
        case 13: return  y - x;  //(-1,1,0) 
        case 14: return -y + z;  //(0,-1,1) 
        case 15: return -y - z;  //(0,-1,-1) 
        default: return 0.0f;
    }
}

class ValueNoise 
{ 
public: 
    ValueNoise(unsigned seed = 2024) 
    { 
        std::mt19937 gen(seed); 
        // floats from 0 to 1
        std::uniform_real_distribution<float> distrFloat;
        // gen is seeded
        auto randFloat = std::bind(distrFloat, gen); 
        // create an array of random values
        for (unsigned k = 0; k < kMaxTableSize; ++k) { 
            // r[k] = randFloat();
            gradients[k] = Vec3f(2 * randFloat() - 1, 2 * randFloat() -1, 2 * randFloat() - 1);
            gradients[k] = gradients[k] * (1 / gradients[k].length());
            permutationTable[k] = k;
        } 

        std::uniform_int_distribution<unsigned> distrUInt;
        // gen is seeded
        auto randUInt = std::bind(distrUInt, gen);
        for (unsigned k=0; k<kMaxTableSize; k++) {
            // modulo kMaxTableSize
            unsigned i = randUInt() & kMaxTableSizeMask;
            // swap to elements to make a more random permutation
            std::swap(permutationTable[k], permutationTable[i]);
            // make the second part of the permutation be the same
            permutationTable[k+kMaxTableSize] = permutationTable[k];
        }
    }

    float eval(const Vec3f &p) const 
    {
        int X = ((int)std::floor(p.x)) & kMaxTableSizeMask; // find cube of point
        int Y = ((int)std::floor(p.y)) & kMaxTableSizeMask;
        int Z = ((int)std::floor(p.z)) & kMaxTableSizeMask;

        float x = p.x - std::floor(p.x);  // find relative x,y,z in cube
        float y = p.y - std::floor(p.y);
        float z = p.z - std::floor(p.z);

        float u = quintic(x);
        float v = quintic(y);
        float w = quintic(z);

        int A  = permutation[X]   + Y;
        int AA = permutation[A]   + Z;
        int AB = permutation[A+1] + Z;
        int B  = permutation[X+1] + Y;
        int BA = permutation[B]   + Z;
        int BB = permutation[B+1] + Z;

        float a = gradient(permutation[AA], x,   y,   z);
        float b = gradient(permutation[BA], x-1, y,   z);
        float c = gradient(permutation[AB], x,   y-1, z);
        float d = gradient(permutation[BB], x-1, y-1, z);

        float e = gradient(permutation[AA+1], x,   y,   z-1);
        float f = gradient(permutation[BA+1], x-1, y,   z-1);
        float g = gradient(permutation[AB+1], x,   y-1, z-1);
        float h = gradient(permutation[BB+1], x-1, y-1, z-1);

        float l1 = lerp(u, a, b);
        float l2 = lerp(u, c, d);
        float l3 = lerp(u, e, f);
        float l4 = lerp(u, g, h);

        float l5 = lerp(v, l1, l2);
        float l6 = lerp(v, l3, l4);

        return lerp(w, l5, l6);
    }

    // float evalDeriv(const Vec3f &p) const
    // { 
    //     // modulo kMaxTableSize, its a power of two
    //     int xi0 = ((int)std::floor(p.x)) & kMaxTableSizeMask; 
    //     int yi0 = ((int)std::floor(p.y)) & kMaxTableSizeMask;
    //     int zi0 = ((int)std::floor(p.z)) & kMaxTableSizeMask;

    //     int xi1 = (xi0 + 1) & kMaxTableSizeMask;
    //     int yi1 = (yi0 + 1) & kMaxTableSizeMask; 
    //     int zi1 = (zi0 + 1) & kMaxTableSizeMask; 

    //     // distance from top left to point
    //     float tx = p.x - ((int)std::floor(p.x)); 
    //     float ty = p.y - ((int)std::floor(p.y));
    //     float tz = p.z - ((int)std::floor(p.z));

    //     // remapping of tx and ty using the Smoothstep function 
    //     float u = quintic(tx); 
    //     float v = quintic(ty); 
    //     float w = quintic(tz); 

    //     // vectors from the grid points to p
    //     float x0 = tx, x1 = tx-1;
    //     float y0 = ty, y1 = ty-1;
    //     float z0 = tz, z1 = tz-1;

    //     // seeded random at each corner
    //     float a = gradient(permutationTable[permutationTable[permutationTable[xi0] + yi0] + zi0], x0, y0, z0); 
    //     float b = gradient(permutationTable[permutationTable[permutationTable[xi1] + yi0] + zi0], x1, y0, z0); 
    //     float c = gradient(permutationTable[permutationTable[permutationTable[xi0] + yi1] + zi0], x0, y1, z0); 
    //     float d = gradient(permutationTable[permutationTable[permutationTable[xi1] + yi1] + zi0], x1, y1, z0); 

    //     float e = gradient(permutationTable[permutationTable[permutationTable[xi0] + yi0] + zi1], x0, y0, z1); 
    //     float f = gradient(permutationTable[permutationTable[permutationTable[xi1] + yi0] + zi1], x1, y0, z1); 
    //     float g = gradient(permutationTable[permutationTable[permutationTable[xi0] + yi1] + zi1], x0, y1, z1); 
    //     float h = gradient(permutationTable[permutationTable[permutationTable[xi1] + yi1] + zi1], x1, y1, z1); 

    //     float du = quinticDeriv(tx); 
    //     float dv = quinticDeriv(ty); 
    //     float dw = quinticDeriv(tz); 
 
    //     float k0 = a; 
    //     float k1 = (b - a); 
    //     float k2 = (c - a); 
    //     float k3 = (e - a); 
    //     float k4 = (a + d - b - c); 
    //     float k5 = (a + f - b - e); 
    //     float k6 = (a + g - c - e); 
    //     float k7 = (b + c + e + h - a - d - f - g); 
 
    //     // derivs.x = du *(k1 + k4 * v + k5 * w + k7 * v * w); 
    //     // derivs.y = dv *(k2 + k4 * u + k6 * w + k7 * v * w); 
    //     // derivs.z = dw *(k3 + k5 * u + k6 * v + k7 * v * w); 
 
    //     return k0 + k1 * u + k2 * v + k3 * w + k4 * u * v + k5 * u * w + k6 * v * w + k7 * u * v * w; 
    // } 

    static const unsigned kMaxTableSize = 256; 
    static const unsigned kMaxTableSizeMask = kMaxTableSize - 1;
    // used to hash into r
    unsigned int permutationTable[2*kMaxTableSize];

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
    // holds random floats in range 0 to 1
    float r[kMaxTableSize]; 
    Vec3f gradients[kMaxTableSize];
}; 

int main(int argc, char **argv) 
{ 
    unsigned imageWidth = 512; 
    unsigned imageHeight = 512; 
    // Using std::vector instead of raw pointer
    std::vector<float> noiseMap(imageWidth * imageHeight); 

    // generate value noise
    ValueNoise noise; 
    float frequency = 1/32.; 
    int sumCycles = 0;
    for (unsigned j = 0; j < imageHeight; ++j) { 
        for (unsigned i = 0; i < imageWidth; ++i) { 
            int a = rdtsc();
            // generate a float in the range [0:1]
            noiseMap[j * imageWidth + i] = noise.eval(Vec3f(i, 0, j) * frequency);
            sumCycles += (rdtsc()-a);
        } 
    } 

    std::cout << "average cycle count per call: " << sumCycles / (imageWidth * imageHeight) << " cycles" << std::endl;

    // output noise map to PPM
    std::ofstream ofs("./noise.ppm", std::ios::out | std::ios::binary);
    if (!ofs) {
        std::cerr << "Failed to open noise.ppm for writing.\n";
        return 1;
    }

    ofs << "P6\n" << imageWidth << " " << imageHeight << "\n255\n"; 
    for (unsigned k = 0; k < imageWidth * imageHeight; ++k) { 
        unsigned char n = static_cast<unsigned char>((noiseMap[k]+1)*127.5); // Corrected cast
        ofs << n << n << n; 
    } 
    ofs.close(); 

    return 0; 
}
