#pragma once

#include "Math.hpp"
#include "Vec4f.hpp"
#include "Vec4i.hpp"

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
    
    void FromVector(const Vec4f& input)
    {
        const Vec4f vAbs = Vec4f::Abs(input);
        Vec4f n = input / (vAbs.x + vAbs.y + vAbs.z);

        if (input.z < 0.0f)
        {
            //(1.0 - abs(v.yx))* (v.xy >= 0.0 ? 1.0 : -1.0);
            n = n.Swizzle<1, 0, 1, 0>();
            n = (Vec4f(1.0f) - Vec4f::Abs(n)).ChangeSign(input < Vec4f::Zero());
        }

        const Vec4i i = Vec4i::Convert(n * Scale);
        u = static_cast<int16>(i.x);
        v = static_cast<int16>(i.y);
    }

    const Vec4f ToVector() const
    {
        Vec4f f = Vec4f::FromIntegers(u, v, 0, 0) * (1.0f / Scale);

        // based on: https://twitter.com/Stubbesaurus/status/937994790553227264

        const Vec4f fAbs = Vec4f::Abs(f);
        f.z = 1.0f - fAbs.x - fAbs.y;

        // t = Max([-f.z, -f.z, 0, 0], 0)
        const Vec4f t = Vec4f::Max(-f.Swizzle<2, 2, 3, 3>(), Vec4f::Zero());

        f += t.ChangeSign(f > Vec4f::Zero());

        return f.Normalized3();
    }

private:
    int16 u;
    int16 v;
};

static_assert(sizeof(PackedUnitVector3) == 4, "Invalid size of PackedUnitVector3");

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

    void FromVector(Vec4f color)
    {
        NFE_ASSERT((color >= Vec4f::Zero()).All(), "Color cannot be negative");

        Vec4f ycocg = color.SplatX() * Vec4f(0.25f, 0.5f * ChromaScale, -0.25f * ChromaScale);
        ycocg = Vec4f::MulAndAdd(color.SplatY(), Vec4f(0.5f, 0.0f, 0.5f * ChromaScale), ycocg);
        ycocg = Vec4f::MulAndAdd(color.SplatZ(), Vec4f(0.25f, -0.5f * ChromaScale, -0.25f * ChromaScale), ycocg);

        y = ycocg.x;

        if (ycocg.x > 0.0f)
        {
            ycocg /= y;
        }

        const Vec4i i = Vec4i::Convert(ycocg);
        co = static_cast<int16>(i.y);
        cg = static_cast<int16>(i.z);
    }

    const Vec4f ToVector() const
    {
        const Vec4f cocg = Vec4f::FromIntegers(co, cg, 0, 0) * (1.0f / ChromaScale);
        const float tmp = 1.0f - cocg.y;
        return Vec4f::Max(Vec4f::Zero(), Vec4f(tmp + cocg.x, 1.0f + cocg.y, tmp - cocg.x) * y);
    }

private:
    float y;
    int16 co;
    int16 cg;
};

static_assert(sizeof(PackedColorRgbHdr) == 8, "Invalid size of PackedColorRgbHdr");

///////////////////////////////////////////////////////////////////////////////////////////////////

// Shared exponent 3-element float, as in DXGI_FORMAT_R9G9B9E5_SHAREDEXP
// 9 bits for exponent and 5 bits for shared exponent
class SharedExpFloat3
{
public:
    NFE_FORCE_INLINE SharedExpFloat3() : v(0) { }
    NFE_FORCE_INLINE explicit SharedExpFloat3(uint32 value) : v(value) { }
    NFE_FORCE_INLINE SharedExpFloat3(const SharedExpFloat3&) = default;
    NFE_FORCE_INLINE SharedExpFloat3& operator = (const SharedExpFloat3&) = default;

    const Vec4f ToVector() const
    {
        Common::FundamentalTypesUnion fi;
        fi.u32 = 0x33800000 + (e << 23);
        return fi.f * Vec4i(x, y, z, 0).ConvertToVec4f();
    }

private:
    union
    {
        struct
        {
            uint32 x : 9; // 'x' mantissa
            uint32 y : 9; // 'y' mantissa
            uint32 z : 9; // 'z' mantissa
            uint32 e : 5; // shared exponent
        };
        uint32 v;
    };
};

static_assert(sizeof(SharedExpFloat3) == 4, "Invalid size of SharedExpFloat3");

///////////////////////////////////////////////////////////////////////////////////////////////////

// Packed 3-channel RGB, as in DXGI_FORMAT_R11G11B10_FLOAT
class PackedFloat3
{
public:
    NFE_FORCE_INLINE PackedFloat3() : v(0) { }
    NFE_FORCE_INLINE explicit PackedFloat3(uint32 value) : v(value) { }
    NFE_FORCE_INLINE PackedFloat3(const PackedFloat3&) = default;
    NFE_FORCE_INLINE PackedFloat3& operator = (const PackedFloat3&) = default;

    const Vec4f ToVector() const
    {
        // TODO handle INF, NAN and denormals?
        uint32 x = ((xe + 112) << 23) | (xm << 17);
        uint32 y = ((ye + 112) << 23) | (ym << 17);
        uint32 z = ((ze + 112) << 23) | (zm << 17);
        return Vec4f(x, y, z, 0u);
    }

private:
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

static_assert(sizeof(PackedFloat3) == 4, "Invalid size of PackedFloat3");

///////////////////////////////////////////////////////////////////////////////////////////////////

struct Packed565
{
public:
    NFE_FORCE_INLINE Packed565() : v(0) { }
    NFE_FORCE_INLINE Packed565(uint8 x, uint8 y, uint8 z) : x(x), y(y), z(z) { }
    NFE_FORCE_INLINE explicit Packed565(uint16 value) : v(value) { }
    NFE_FORCE_INLINE Packed565(const Packed565&) = default;
    NFE_FORCE_INLINE Packed565& operator = (const Packed565&) = default;

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

static_assert(sizeof(Packed565) == 2, "Invalid size of Packed565");


} // namespace Math
} // namespace NFE
