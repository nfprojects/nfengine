#pragma once

#include "Math.hpp"

namespace NFE {
namespace Math {


// 4-byte compressed 3D unit vector
// Based on "Signed Octahedron Normal Encoding" scheme:
// the vector is first mapped to surface of octahedron and then to [-1,1] square
// Note: accuracy somewhere between Vec3f and Half3
class PackedUnitVector3
{
public:
    static constexpr float Scale = 32767.0f;

    NFE_FORCE_INLINE PackedUnitVector3() : u(0), v(0) { }
    NFE_FORCE_INLINE PackedUnitVector3(int16 u, int16 v) : u(u), v(v) { }
    NFE_FORCE_INLINE PackedUnitVector3(const PackedUnitVector3&) = default;
    NFE_FORCE_INLINE PackedUnitVector3& operator = (const PackedUnitVector3&) = default;
    
    NFCOMMON_API static const PackedUnitVector3 FromVector(const Vec4f& input);

    int16 u;
    int16 v;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

// HDR color packed to 8 bytes
// luminance has full precission, chroma has 16 bit precission per channel
class PackedColorRgbHdr
{
public:
    static constexpr float ChromaScale = 16383.0f;

    NFE_FORCE_INLINE PackedColorRgbHdr() : y(0.0f), co(0), cg(0) { }
    NFE_FORCE_INLINE PackedColorRgbHdr(const PackedColorRgbHdr&) = default;
    NFE_FORCE_INLINE PackedColorRgbHdr& operator = (const PackedColorRgbHdr&) = default;

    NFCOMMON_API static const PackedColorRgbHdr FromVector(const Vec4f& vec);

    float y;
    int16 co;
    int16 cg;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

// Shared exponent 3-element unsigned float, as in DXGI_FORMAT_R9G9B9E5_SHAREDEXP
// 9 bits for exponent and 5 bits for shared exponent
class PackedUFloat3_9_9_9_5
{
public:
    NFE_FORCE_INLINE PackedUFloat3_9_9_9_5() : v(0) { }
    NFE_FORCE_INLINE explicit PackedUFloat3_9_9_9_5(uint32 value) : v(value) { }
    NFE_FORCE_INLINE PackedUFloat3_9_9_9_5(const PackedUFloat3_9_9_9_5&) = default;
    NFE_FORCE_INLINE PackedUFloat3_9_9_9_5& operator = (const PackedUFloat3_9_9_9_5&) = default;

    NFCOMMON_API static const PackedUFloat3_9_9_9_5 FromVector(const Vec4f& vec);

    union
    {
        struct
        {
            uint32 mx : 9; // 'x' mantissa
            uint32 my : 9; // 'y' mantissa
            uint32 mz : 9; // 'z' mantissa
            uint32 e : 5; // shared exponent
        };
        uint32 v;
    };
};

///////////////////////////////////////////////////////////////////////////////////////////////////

// Packed 3-element unsigned float, as in DXGI_FORMAT_R11G11B10_FLOAT
class PackedUFloat3_11_11_10
{
public:
    NFE_FORCE_INLINE PackedUFloat3_11_11_10() : v(0) { }
    NFE_FORCE_INLINE explicit PackedUFloat3_11_11_10(uint32 value) : v(value) { }
    NFE_FORCE_INLINE PackedUFloat3_11_11_10(const PackedUFloat3_11_11_10&) = default;
    NFE_FORCE_INLINE PackedUFloat3_11_11_10& operator = (const PackedUFloat3_11_11_10&) = default;

    union
    {
        struct
        {
            uint32 xm : 6; // x-mantissa
            uint32 xe : 5; // x-exponent
            uint32 ym : 6; // y-mantissa
            uint32 ye : 5; // y-exponent
            uint32 zm : 5; // z-mantissa
            uint32 ze : 5; // z-exponent
        };
        uint32 v;
    };
};

///////////////////////////////////////////////////////////////////////////////////////////////////

struct Packed_5_6_5
{
public:
    NFE_FORCE_INLINE Packed_5_6_5() : v(0) { }
    NFE_FORCE_INLINE Packed_5_6_5(uint8 x, uint8 y, uint8 z) : x(x), y(y), z(z) { }
    NFE_FORCE_INLINE explicit Packed_5_6_5(uint16 value) : v(value) { }
    NFE_FORCE_INLINE Packed_5_6_5(const Packed_5_6_5&) = default;
    NFE_FORCE_INLINE Packed_5_6_5& operator = (const Packed_5_6_5&) = default;

    union
    {
        struct
        {
            uint16 x : 5;
            uint16 y : 6;
            uint16 z : 5;
        };
        uint16 v;
    };
};

///////////////////////////////////////////////////////////////////////////////////////////////////

struct Packed_4_4_4_4
{
public:
    NFE_FORCE_INLINE Packed_4_4_4_4() : v(0) { }
    NFE_FORCE_INLINE Packed_4_4_4_4(uint8 x, uint8 y, uint8 z, uint8 w) : x(x), y(y), z(z), w(w) { }
    NFE_FORCE_INLINE explicit Packed_4_4_4_4(uint16 value) : v(value) { }
    NFE_FORCE_INLINE Packed_4_4_4_4(const Packed_4_4_4_4&) = default;
    NFE_FORCE_INLINE Packed_4_4_4_4& operator = (const Packed_4_4_4_4&) = default;

    union
    {
        struct
        {
            uint8 x : 4;
            uint8 y : 4;
            uint8 z : 4;
            uint8 w : 4;
        };
        uint16 v;
    };
};

///////////////////////////////////////////////////////////////////////////////////////////////////

struct Packed_10_10_10_2
{
public:
    NFE_FORCE_INLINE Packed_10_10_10_2() : v(0) { }
    NFE_FORCE_INLINE Packed_10_10_10_2(uint16 x, uint16 y, uint16 z, uint16 w) : x(x), y(y), z(z), w(w) { }
    NFE_FORCE_INLINE Packed_10_10_10_2(const Packed_10_10_10_2&) = default;
    NFE_FORCE_INLINE Packed_10_10_10_2& operator = (const Packed_10_10_10_2&) = default;

    union
    {
        struct
        {
            uint32 x : 10;
            uint32 y : 10;
            uint32 z : 10;
            uint32 w : 2;
        };
        uint32 v;
    };
};

} // namespace Math
} // namespace NFE
