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

    union f32_4x
    {
        __m128 sse;
        f32 E[4];
    };

    union u32_4x {
        __m128i sse;
        u32 E[4];
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

    inline
    f32_4x _select(u32_4x mask, f32_4x A, f32_4x B) {
        f32_4x result;
        result.sse = _mm_blendv_ps(B.sse, A.sse, mask.sse);;
        return result;
    }

#else

    // #include <mach/mach_time.h>
    // static inline uint64_t read_cycle_counter() {
    //     return mach_absolute_time();  // Use macOS-specific high-resolution timer
    // }

    // different representations of the same thing
    union f32_4x
    {
        float32x4_t sse;
        f32 E[4];
    };

    union u32_4x {
        uint32x4_t sse;
        u32 E[4];
    };

    f32_4x F32_4X(f32 A, f32 B, f32 C, f32 D) {
        // no need to set backwards
        f32_4x result = { A, B, C, D };
        return result;
    }

    f32_4x F32_4X(f32 A) {
        // set backwards
        f32_4x result = { vdupq_n_f32(A) };
        return result;
    }

    f32_4x F32_4X(float32x4_t A) {

        f32_4x res;
        res.sse = A;
        return res;
    }

    u32_4x U32_4X(u32 A, u32 B, u32 C, u32 D) {
        // no need to set backwards
        u32_4x result = { A, B, C, D };
        return result;
    }

    u32_4x U32_4X(u32 A) {
        // set backwards
        u32_4x result = { vdupq_n_u32(A) };
        return result;
    }

    u32_4x U32_4X(uint32x4_t A) {

        u32_4x res;
        res.sse = A;
        return res;
    }

    inline f32_4x
    operator+ (f32_4x A, f32_4x B)
    {
        f32_4x result = { vaddq_f32(A.sse, B.sse) };
        return result;
    }

    inline f32_4x
    operator- (f32_4x A, f32_4x B)
    {
        f32_4x result = { vsubq_f32(A.sse, B.sse) };
        return result;
    }

    inline f32_4x
    operator* (f32_4x A, f32_4x B)
    {
        f32_4x result = { vmulq_f32(A.sse, B.sse) };
        return result;
    }

    inline f32_4x
    operator/ (f32_4x A, f32_4x B)
    {
        f32_4x result = { vdivq_f32(A.sse, B.sse) };
        return result;
    } 

    inline u32_4x
    operator* (u32_4x A, u32_4x B)
    {
        u32_4x result = { vmulq_u32(A.sse, B.sse) };
        return result;
    }

    inline u32_4x
    operator& (u32_4x A, u32_4x B)
    {
        u32_4x result = { vandq_u32(A.sse, B.sse) };
        return result;
    }

    inline u32_4x
    operator| (u32_4x A, u32_4x B)
    {
        u32_4x result = { vorrq_u32(A.sse, B.sse) };
        return result;
    }

    inline u32_4x
    operator==(u32_4x A, u32_4x B)
    {
        u32_4x result = { vceqq_f32(A.sse, B.sse) };
        return result;
    }

    inline u32_4x
    operator>(u32_4x A, u32_4x B)
    {
        u32_4x result = { vcgtq_f32(A.sse, B.sse) };
        return result;
    }

    inline u32_4x
    operator<(u32_4x A, u32_4x B)
    {
        u32_4x result = { vcltq_f32(A.sse, B.sse) };
        return result;
    }

    inline f32_4x
    _select(u32_4x mask, f32_4x A, f32_4x B) {
        f32_4x result;
        result.sse = vbslq_f32(mask.sse, A.sse, B.sse);;
        return result;
    }

#endif