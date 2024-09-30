#include "perlin_sse.h"
#include <emscripten/bind.h>
#include <emscripten/val.h>

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

f32_4x _127p5f = F32_4X(127.5);
f32_4x _15f = F32_4X(15);
f32_4x _10f = F32_4X(10);
f32_4x _6f  = F32_4X(6);
f32_4x _2f  = F32_4X(2);
f32_4x _1f  = F32_4X(1);
f32_4x _n1f = F32_4X(-1);

// pack scalars
u32_4x _15u = U32_4X(15);
u32_4x _14u = U32_4X(14);
u32_4x _12u = U32_4X(12);
u32_4x _8u  = U32_4X(8);
u32_4x _4u  = U32_4X(4);
u32_4x _2u  = U32_4X(2);
u32_4x _1u  = U32_4X(1);

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

f32_4x quintic_4x(const f32_4x &t) {

    return t * t * t * (t * (t * _6f - _15f) + _10f);
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

f32_4x gradient_4x (u32_4x hash, f32_4x x, f32_4x y, f32_4x z) {

    auto h = hash & _15u;

    auto uSel = h < _8u;
    auto vSel = h < _4u;
    auto xSel = (h == _12u | h == _14u);

    // selects either x or y based on interpolation via uSel
    auto u  = _select(uSel, x, y);
    auto xz = _select(xSel, x, z);
    auto v  = _select(vSel, y, xz);

    auto uFlip = (h & _1u) == _1u;
    auto vFlip = (h & _2u) == _2u;

    auto R0 = _select(uFlip, _n1f*u, u);
    auto R1 = _select(vFlip, _n1f*v, v);
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

    u32_4x H0246 = U32_4X(H0, H2, H4, H6);
    u32_4x H1357 = U32_4X(H1, H3, H5, H7);

    f32_4x A4 = gradient_4x(H0246, x_x_x_x, y_ny_y_ny, z_z_nz_nz);
    f32_4x B4 = gradient_4x(H1357, nx_nx_nx_nx, y_ny_y_ny, z_z_nz_nz);

    f32_4x u_4x = F32_4X(u);
    f32_4x v_4x = F32_4X(v);

    f32_4x L0123 = lerp_4x(u_4x, A4, B4);

    f32_4x L02 = F32_4X(L0123.E[0], L0123.E[2], 0, 0);
    f32_4x L13 = F32_4X(L0123.E[1], L0123.E[3], 0, 0);

    f32_4x L5656 = lerp_4x(v_4x, L02, L13);
    f32 res = lerp(w, L5656.E[0], L5656.E[1]);

    return res;
}

void perlinNoiseSIMD_4x(const f32 x, const f32 y, const f32 z, const f32 f, uint8_t *data) 
{

    u32_4x mask = U32_4X(kMaxTableSizeMask);

    f32_4x X_4x = F32_4X(x, x+f, x+f+f, x+f+f+f);
    f32_4x Y_4x = F32_4X(y);
    f32_4x Z_4x = F32_4X(z);

    u32_4x XI_4x = U32_4X(vcvtq_u32_f32(X_4x.sse));
    u32_4x YI_4x = U32_4X(vcvtq_u32_f32(Y_4x.sse));
    u32_4x ZI_4x = U32_4X(vcvtq_u32_f32(Z_4x.sse));

    u32_4x XM_4x = XI_4x & mask;
    u32_4x YM_4x = YI_4x & mask;
    u32_4x ZM_4x = ZI_4x & mask;

    // x - std::floor(x)
    f32_4x SX_4x = X_4x - F32_4X(vcvtq_f32_u32(XI_4x.sse));
    f32_4x SY_4x = Y_4x - F32_4X(vcvtq_f32_u32(YI_4x.sse));
    f32_4x SZ_4x = Z_4x - F32_4X(vcvtq_f32_u32(ZI_4x.sse));

    // (x - std::floor(x)) - 1
    f32_4x NSX_4x = SX_4x - _1f;
    f32_4x NSY_4x = SY_4x - _1f;
    f32_4x NSZ_4x = SZ_4x - _1f;

    f32_4x u_4x = quintic_4x(SX_4x);
    f32_4x v_4x = quintic_4x(SY_4x);
    f32_4x w_4x = quintic_4x(SZ_4x);

    u32 A_0  = permutation[XM_4x.E[0]]   + Y_4x.E[0];
    u32 AA_0 = permutation[A_0]          + Z_4x.E[0];
    u32 AB_0 = permutation[A_0+1]        + Z_4x.E[0];
    u32 B_0  = permutation[XM_4x.E[0]+1] + Y_4x.E[0];
    u32 BA_0 = permutation[B_0]          + Z_4x.E[0];
    u32 BB_0 = permutation[B_0+1]        + Z_4x.E[0];

    u32 H0_0 = permutation[AA_0];
    u32 H1_0 = permutation[BA_0];
    u32 H2_0 = permutation[AB_0];
    u32 H3_0 = permutation[BB_0];
    u32 H4_0 = permutation[AA_0+1];
    u32 H5_0 = permutation[BA_0+1];
    u32 H6_0 = permutation[AB_0+1];
    u32 H7_0 = permutation[BB_0+1];


    u32 A_1  = permutation[XM_4x.E[1]]   + Y_4x.E[1];
    u32 AA_1 = permutation[A_1]          + Z_4x.E[1];
    u32 AB_1 = permutation[A_1+1]        + Z_4x.E[1];
    u32 B_1  = permutation[XM_4x.E[1]+1] + Y_4x.E[1];
    u32 BA_1 = permutation[B_1]          + Z_4x.E[1];
    u32 BB_1 = permutation[B_1+1]        + Z_4x.E[1];

    u32 H0_1 = permutation[AA_1];
    u32 H1_1 = permutation[BA_1];
    u32 H2_1 = permutation[AB_1];
    u32 H3_1 = permutation[BB_1];
    u32 H4_1 = permutation[AA_1+1];
    u32 H5_1 = permutation[BA_1+1];
    u32 H6_1 = permutation[AB_1+1];
    u32 H7_1 = permutation[BB_1+1];


    u32 A_2  = permutation[XM_4x.E[2]]   + Y_4x.E[2];
    u32 AA_2 = permutation[A_2]          + Z_4x.E[2];
    u32 AB_2 = permutation[A_2+1]        + Z_4x.E[2];
    u32 B_2  = permutation[XM_4x.E[2]+1] + Y_4x.E[2];
    u32 BA_2 = permutation[B_2]          + Z_4x.E[2];
    u32 BB_2 = permutation[B_2+1]        + Z_4x.E[2];

    u32 H0_2 = permutation[AA_2];
    u32 H1_2 = permutation[BA_2];
    u32 H2_2 = permutation[AB_2];
    u32 H3_2 = permutation[BB_2];
    u32 H4_2 = permutation[AA_2+1];
    u32 H5_2 = permutation[BA_2+1];
    u32 H6_2 = permutation[AB_2+1];
    u32 H7_2 = permutation[BB_2+1];


    u32 A_3  = permutation[XM_4x.E[3]]   + Y_4x.E[3];
    u32 AA_3 = permutation[A_3]          + Z_4x.E[3];
    u32 AB_3 = permutation[A_3+1]        + Z_4x.E[3];
    u32 B_3  = permutation[XM_4x.E[3]+1] + Y_4x.E[3];
    u32 BA_3 = permutation[B_3]          + Z_4x.E[3];
    u32 BB_3 = permutation[B_3+1]        + Z_4x.E[3];

    u32 H0_3 = permutation[AA_3];
    u32 H1_3 = permutation[BA_3];
    u32 H2_3 = permutation[AB_3];
    u32 H3_3 = permutation[BB_3];
    u32 H4_3 = permutation[AA_3+1];
    u32 H5_3 = permutation[BA_3+1];
    u32 H6_3 = permutation[AB_3+1];
    u32 H7_3 = permutation[BB_3+1];


    u32_4x H0 = U32_4X (H0_0, H0_1, H0_2, H0_3);
    u32_4x H1 = U32_4X (H1_0, H1_1, H1_2, H1_3);
    u32_4x H2 = U32_4X (H2_0, H2_1, H2_2, H2_3);
    u32_4x H3 = U32_4X (H3_0, H3_1, H3_2, H3_3);
    u32_4x H4 = U32_4X (H4_0, H4_1, H4_2, H4_3);
    u32_4x H5 = U32_4X (H5_0, H5_1, H5_2, H5_3);
    u32_4x H6 = U32_4X (H6_0, H6_1, H6_2, H6_3);
    u32_4x H7 = U32_4X (H7_0, H7_1, H7_2, H7_3);


    f32_4x G0 = gradient_4x(H0, SX_4x,  SY_4x,  SZ_4x); 
    f32_4x G1 = gradient_4x(H1, NSX_4x, SY_4x,  SZ_4x); 
    f32_4x G2 = gradient_4x(H2, SX_4x,  NSY_4x, SZ_4x); 
    f32_4x G3 = gradient_4x(H3, NSX_4x, NSY_4x, SZ_4x); 

    f32_4x G4 = gradient_4x(H4, SX_4x,  SY_4x,  NSZ_4x); 
    f32_4x G5 = gradient_4x(H5, NSX_4x, SY_4x,  NSZ_4x); 
    f32_4x G6 = gradient_4x(H6, SX_4x,  NSY_4x, NSZ_4x); 
    f32_4x G7 = gradient_4x(H7, NSX_4x, NSY_4x, NSZ_4x); 


    f32_4x L0 = lerp_4x(u_4x, G0, G1);
    f32_4x L1 = lerp_4x(u_4x, G2, G3);

    f32_4x L2 = lerp_4x(u_4x, G4, G5);
    f32_4x L3 = lerp_4x(u_4x, G6, G7);

    f32_4x L5 = lerp_4x(v_4x, L0, L1);
    f32_4x L6 = lerp_4x(v_4x, L2, L3);

    f32_4x result = lerp_4x(w_4x, L5, L6);
    result = (result + _1f) * _127p5f;

    data[0] = std::floor(result.E[0]);
    data[1] = std::floor(result.E[1]);
    data[2] = std::floor(result.E[2]);
    data[3] = std::floor(result.E[3]);
}

void perlinNoiseSIMD_8x(const f32 x, const f32 y, const f32 z, const f32 f, f32 *data) 
{

    u32_4x mask = U32_4X(kMaxTableSizeMask);

    f32_4x X_4x = F32_4X(x, x+f, x+f+f, x+f+f+f);
    f32_4x Y_4x = F32_4X(y);
    f32_4x Z_4x = F32_4X(z);

    u32_4x XI_4x = U32_4X(vcvtq_u32_f32(X_4x.sse));
    u32_4x YI_4x = U32_4X(vcvtq_u32_f32(Y_4x.sse));
    u32_4x ZI_4x = U32_4X(vcvtq_u32_f32(Z_4x.sse));

    u32_4x XM_4x = XI_4x & mask;
    u32_4x YM_4x = YI_4x & mask;
    u32_4x ZM_4x = ZI_4x & mask;

    // x - std::floor(x)
    f32_4x SX_4x = X_4x - F32_4X(vcvtq_f32_u32(XI_4x.sse));
    f32_4x SY_4x = Y_4x - F32_4X(vcvtq_f32_u32(YI_4x.sse));
    f32_4x SZ_4x = Z_4x - F32_4X(vcvtq_f32_u32(ZI_4x.sse));

    // (x - std::floor(x)) - 1
    f32_4x NSX_4x = SX_4x - _1f;
    f32_4x NSY_4x = SY_4x - _1f;
    f32_4x NSZ_4x = SZ_4x - _1f;

    f32_4x u_4x = quintic_4x(SX_4x);
    f32_4x v_4x = quintic_4x(SY_4x);
    f32_4x w_4x = quintic_4x(SZ_4x);

    u32 A_0  = permutation[XM_4x.E[0]]   + Y_4x.E[0];
    u32 AA_0 = permutation[A_0]          + Z_4x.E[0];
    u32 AB_0 = permutation[A_0+1]        + Z_4x.E[0];
    u32 B_0  = permutation[XM_4x.E[0]+1] + Y_4x.E[0];
    u32 BA_0 = permutation[B_0]          + Z_4x.E[0];
    u32 BB_0 = permutation[B_0+1]        + Z_4x.E[0];

    u32 H0_0 = permutation[AA_0];
    u32 H1_0 = permutation[BA_0];
    u32 H2_0 = permutation[AB_0];
    u32 H3_0 = permutation[BB_0];
    u32 H4_0 = permutation[AA_0+1];
    u32 H5_0 = permutation[BA_0+1];
    u32 H6_0 = permutation[AB_0+1];
    u32 H7_0 = permutation[BB_0+1];


    u32 A_1  = permutation[XM_4x.E[1]]   + Y_4x.E[1];
    u32 AA_1 = permutation[A_1]          + Z_4x.E[1];
    u32 AB_1 = permutation[A_1+1]        + Z_4x.E[1];
    u32 B_1  = permutation[XM_4x.E[1]+1] + Y_4x.E[1];
    u32 BA_1 = permutation[B_1]          + Z_4x.E[1];
    u32 BB_1 = permutation[B_1+1]        + Z_4x.E[1];

    u32 H0_1 = permutation[AA_1];
    u32 H1_1 = permutation[BA_1];
    u32 H2_1 = permutation[AB_1];
    u32 H3_1 = permutation[BB_1];
    u32 H4_1 = permutation[AA_1+1];
    u32 H5_1 = permutation[BA_1+1];
    u32 H6_1 = permutation[AB_1+1];
    u32 H7_1 = permutation[BB_1+1];


    u32 A_2  = permutation[XM_4x.E[2]]   + Y_4x.E[2];
    u32 AA_2 = permutation[A_2]          + Z_4x.E[2];
    u32 AB_2 = permutation[A_2+1]        + Z_4x.E[2];
    u32 B_2  = permutation[XM_4x.E[2]+1] + Y_4x.E[2];
    u32 BA_2 = permutation[B_2]          + Z_4x.E[2];
    u32 BB_2 = permutation[B_2+1]        + Z_4x.E[2];

    u32 H0_2 = permutation[AA_2];
    u32 H1_2 = permutation[BA_2];
    u32 H2_2 = permutation[AB_2];
    u32 H3_2 = permutation[BB_2];
    u32 H4_2 = permutation[AA_2+1];
    u32 H5_2 = permutation[BA_2+1];
    u32 H6_2 = permutation[AB_2+1];
    u32 H7_2 = permutation[BB_2+1];


    u32 A_3  = permutation[XM_4x.E[3]]   + Y_4x.E[3];
    u32 AA_3 = permutation[A_3]          + Z_4x.E[3];
    u32 AB_3 = permutation[A_3+1]        + Z_4x.E[3];
    u32 B_3  = permutation[XM_4x.E[3]+1] + Y_4x.E[3];
    u32 BA_3 = permutation[B_3]          + Z_4x.E[3];
    u32 BB_3 = permutation[B_3+1]        + Z_4x.E[3];

    u32 H0_3 = permutation[AA_3];
    u32 H1_3 = permutation[BA_3];
    u32 H2_3 = permutation[AB_3];
    u32 H3_3 = permutation[BB_3];
    u32 H4_3 = permutation[AA_3+1];
    u32 H5_3 = permutation[BA_3+1];
    u32 H6_3 = permutation[AB_3+1];
    u32 H7_3 = permutation[BB_3+1];


    u32_4x H0 = U32_4X (H0_0, H0_1, H0_2, H0_3);
    u32_4x H1 = U32_4X (H1_0, H1_1, H1_2, H1_3);
    u32_4x H2 = U32_4X (H2_0, H2_1, H2_2, H2_3);
    u32_4x H3 = U32_4X (H3_0, H3_1, H3_2, H3_3);
    u32_4x H4 = U32_4X (H4_0, H4_1, H4_2, H4_3);
    u32_4x H5 = U32_4X (H5_0, H5_1, H5_2, H5_3);
    u32_4x H6 = U32_4X (H6_0, H6_1, H6_2, H6_3);
    u32_4x H7 = U32_4X (H7_0, H7_1, H7_2, H7_3);


    f32_4x G0 = gradient_4x(H0, SX_4x,  SY_4x,  SZ_4x); 
    f32_4x G1 = gradient_4x(H1, NSX_4x, SY_4x,  SZ_4x); 
    f32_4x G2 = gradient_4x(H2, SX_4x,  NSY_4x, SZ_4x); 
    f32_4x G3 = gradient_4x(H3, NSX_4x, NSY_4x, SZ_4x); 

    f32_4x G4 = gradient_4x(H4, SX_4x,  SY_4x,  NSZ_4x); 
    f32_4x G5 = gradient_4x(H5, NSX_4x, SY_4x,  NSZ_4x); 
    f32_4x G6 = gradient_4x(H6, SX_4x,  NSY_4x, NSZ_4x); 
    f32_4x G7 = gradient_4x(H7, NSX_4x, NSY_4x, NSZ_4x); 


    f32_4x L0 = lerp_4x(u_4x, G0, G1);
    f32_4x L1 = lerp_4x(u_4x, G2, G3);

    f32_4x L2 = lerp_4x(u_4x, G4, G5);
    f32_4x L3 = lerp_4x(u_4x, G6, G7);

    f32_4x L5 = lerp_4x(v_4x, L0, L1);
    f32_4x L6 = lerp_4x(v_4x, L2, L3);

    f32_4x result = lerp_4x(w_4x, L5, L6);

    data[0] = result.E[0];
    data[1] = result.E[1];
    data[2] = result.E[2];
    data[3] = result.E[3];
}


void perlinNoise_8x(f32 x, f32 y, f32 z, f32 *data, f32 f) 
{
    for (u32 i=0; i<8; i++) {
        data[i] = perlinNoiseSIMD(Vec3f(x+i,y,z)*f);
    }
}

int main (int argc, char** argv) {

    if (argc != 5) { std::cerr << "Usage: " << argv[0] << " <startX> <startY> <width> <height>" << std::endl; return 1; }

    u32 startX = std::atoi(argv[1]);
    u32 startY = std::atoi(argv[2]); 
    u32 width  = std::atoi(argv[3]);
    u32 height = std::atoi(argv[4]); 
    // width * height
    uint8_t* noise = (uint8_t*)malloc(width * height);

    // frequency
    f32 f = 1/32.; 

    auto start = std::chrono::high_resolution_clock::now();

    for (unsigned j = 0; j < width; ++j) { 
        for (unsigned i = 0; i < height; i+=8) { 
            for (u32 k=0; k<8; k+=4) {
                perlinNoiseSIMD_4x((i+startX+k)*f, 0.0f, (j+startY)*f, f, noise + (j * height + i + k));
            }
        } 
    } 

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "took " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "millis" << std::endl;


    std::vector<unsigned char> imageBuffer(width * height * 3);

    for (unsigned k = 0; k < width * height; ++k) { 
        // Normalize noise[k] from [-1,1] to [0,255]
        unsigned char n = static_cast<unsigned char>((noise[k]+1) * 127.5); 
        imageBuffer[3 * k + 0] = n; // Red channel
        imageBuffer[3 * k + 1] = n; // Green channel
        imageBuffer[3 * k + 2] = n; // Blue channel
    }

    std::string outputFilename = "./resources/noise/noise.png";
    // overwrite if already exists
    if (stbi_write_png(outputFilename.c_str(), width, height, 3, imageBuffer.data(), width * 3)) {
        std::cout << "Successfully wrote " << outputFilename << "\n";
    } else {
        std::cerr << "Failed to write " << outputFilename << "\n";
        return 1;
    }
}

extern "C" {

    void generateNoise (u32 startX, u32 startY, u32 startZ, u32 width, u32 height, uint8_t* noise) {

        // frequency
        f32 f = 1/64.; 

        for (unsigned j = 0; j < width; ++j) { 
            for (unsigned i = 0; i < height; i+=8) { 
                for (u32 k=0; k<8; k+=4) {
                    perlinNoiseSIMD_4x((i+startX+k)*f, startY*f, (j+startZ)*f, f, noise + (j * height + i + k));
                }
            } 
        } 
    }

    // Wrapper for malloc
    void* my_malloc(size_t size) {
        return malloc(size);
    }

    // Wrapper for free
    void my_free(void* ptr) {
        free(ptr);
    }
}

// emcc worker2.cpp -O3 -msimd128 -o ../../temp/workerWasm.js \
//   -s EXPORTED_FUNCTIONS="['_generateNoise', '_my_malloc', '_my_free']" \
//   -s EXPORTED_RUNTIME_METHODS="['cwrap', 'ccall']" \
//   -s MODULARIZE=1 \
//   -s EXPORT_NAME='createModule'