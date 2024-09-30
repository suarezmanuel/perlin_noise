#include <cstdio> 
#include <random> 
#include <functional> 
#include <iostream> 
#include <fstream> 
#include <cmath> 
#include <vector> // Added for std::vector
#include <stdint.h>
#include <filesystem>
#include <arm_neon.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../include/stb_image_write.h"

typedef float f32;
typedef uint32_t u32;
typedef int32_t s32;
static const unsigned kMaxTableSize = 256; 
static const unsigned kMaxTableSizeMask = kMaxTableSize - 1;

// https://stackoverflow.com/questions/13772567/how-to-get-the-cpu-cycle-count-in-x86-64-from-c
#ifdef _WIN32

    #include <smmintrin.h>
    #include <emmintrin.h>
    #include <xmmintrin.h>
    #include <intrin.h>
    uint64_t read_cycle_counter(){
        // Serialize with CPUID
        int cpuInfo[4];
        __cpuid(cpuInfo, 0);
        return __rdtsc();
    }

    union f32_8x
    {
        __m128 sse;
        f32 E[4];
    };

    union u32_8x {
        __m128i sse;
        u32 E[4];
    };

    f32_8x F32_8X(f32 A, f32 B, f32 C, f32 D) {
        // set backwards
        f32_8x result = {{ _mm256_set_ps(D,C,B,A) }};
        return result;
    }

    f32_8x F32_8X(f32 A) {
        // set backwards
        f32_8x result = {{ _mm256_set_ps1(A) }};
        return result;
    }

    inline f32_8x
    operator+ (f32_8x A, f32_8x B)
    {
        f32_8x result = {{ _mm256_add_ps(A.sse, B.sse) }};
        return result;
    }

    inline f32_8x
    operator- (f32_8x A, f32_8x B)
    {
        f32_8x result = {{ _mm256_sub_ps(A.sse, B.sse) }};
        return result;
    }

    inline f32_8x
    operator* (f32_8x A, f32_8x B)
    {
        f32_8x result = {{ _mm256_mul_ps(A.sse, B.sse) }};
        return result;
    }

    inline f32_8x
    operator& (f32_8x A, f32_8x B)
    {
        f32_8x result = {{ _mm256_and_ps(A.sse, B.sse) }};
        return result;
    }

    inline f32_8x
    operator| (f32_8x A, f32_8x B)
    {
        f32_8x result = {{ _mm256_or_ps(A.sse, B.sse) }};
        return result;
    }

    inline f32_8x
    operator==(f32_8x A, f32_8x B)
    {
    f32_8x result = {{ _mm256_cmpeq_ps(A.sse, B.sse) }};
    return result;
    }

    inline f32_8x
    operator>(f32_8x A, f32_8x B)
    {
    f32_8x result = {{ _mm256_cmpgt_ps(A.sse, B.sse) }};
    return result;
    }

    inline f32_8x
    operator<(f32_8x A, f32_8x B)
    {
    f32_8x result = {{ _mm256_cmplt_ps(A.sse, B.sse) }};
    return result;
    }

    inline
    f32_8x _select(u32_8x mask, f32_8x A, f32_8x B) {
        f32_8x result;
        result.sse = _mm256_blendv_ps(B.sse, A.sse, mask.sse);;
        return result;
    }

#else

    #include <mach/mach_time.h>
    static inline uint64_t read_cycle_counter() {
        return mach_absolute_time();  // Use macOS-specific high-resolution timer
    }

    // different representations of the same thing
    union f32_8x
    {
        float32x4_t sse;
        f32 E[4];
    };

    union u32_8x {
        uint32x4_t sse;
        u32 E[4];
    };

    f32_8x F32_8X(f32 A, f32 B, f32 C, f32 D) {
        // no need to set backwards
        f32_8x result = { A, B, C, D };
        return result;
    }

    f32_8x F32_8X(f32 A) {
        // set backwards
        f32_8x result = { vdupq_n_f32(A) };
        return result;
    }

    f32_8x F32_8X(float32x4_t A) {

        f32_8x res;
        res.sse = A;
        return res;
    }

    u32_8x U32_8X(u32 A, u32 B, u32 C, u32 D) {
        // no need to set backwards
        u32_8x result = { A, B, C, D };
        return result;
    }

    u32_8x U32_8X(u32 A) {
        // set backwards
        u32_8x result = { vdupq_n_u32(A) };
        return result;
    }

    u32_8x U32_8X(uint32x4_t A) {

        u32_8x res;
        res.sse = A;
        return res;
    }

    inline f32_8x
    operator+ (f32_8x A, f32_8x B)
    {
        f32_8x result = { vaddq_f32(A.sse, B.sse) };
        return result;
    }

    inline f32_8x
    operator- (f32_8x A, f32_8x B)
    {
        f32_8x result = { vsubq_f32(A.sse, B.sse) };
        return result;
    }

    inline f32_8x
    operator* (f32_8x A, f32_8x B)
    {
        f32_8x result = { vmulq_f32(A.sse, B.sse) };
        return result;
    }

    inline f32_8x
    operator/ (f32_8x A, f32_8x B)
    {
        f32_8x result = { vdivq_f32(A.sse, B.sse) };
        return result;
    } 

    inline u32_8x
    operator* (u32_8x A, u32_8x B)
    {
        u32_8x result = { vmulq_u32(A.sse, B.sse) };
        return result;
    }

    inline u32_8x
    operator& (u32_8x A, u32_8x B)
    {
        u32_8x result = { vandq_u32(A.sse, B.sse) };
        return result;
    }

    inline u32_8x
    operator| (u32_8x A, u32_8x B)
    {
        u32_8x result = { vorrq_u32(A.sse, B.sse) };
        return result;
    }

    inline u32_8x
    operator==(u32_8x A, u32_8x B)
    {
        u32_8x result = { vceqq_f32(A.sse, B.sse) };
        return result;
    }

    inline u32_8x
    operator>(u32_8x A, u32_8x B)
    {
        u32_8x result = { vcgtq_f32(A.sse, B.sse) };
        return result;
    }

    inline u32_8x
    operator<(u32_8x A, u32_8x B)
    {
        u32_8x result = { vcltq_f32(A.sse, B.sse) };
        return result;
    }

    inline f32_8x
    _select(u32_8x mask, f32_8x A, f32_8x B) {
        f32_8x result;
        result.sse = vbslq_f32(mask.sse, A.sse, B.sse);;
        return result;
    }

#endif