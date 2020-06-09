#pragma once

#include "Vec4f.hpp"

namespace NFE {
namespace Math {

VecBool4f::VecBool4f(bool scalar)
    : b{ scalar, scalar, scalar, scalar}
{}

VecBool4f::VecBool4f(bool x, bool y, bool z, bool w)
    : b{ x, y, z, w }
{}

template<uint32 index>
bool VecBool4f::Get() const
{
    static_assert(index < 4, "Invalid index");
    return b[index];
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const VecBool4f VecBool4f::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return { b[ix], b[iy], b[iz], b[iw] };
}

// combine into 4-bit mask
int32 VecBool4f::GetMask() const
{
    int32 result = 0;
    result |= b[0] ? 1 : 0;
    result |= b[1] ? 2 : 0;
    result |= b[2] ? 4 : 0;
    result |= b[3] ? 8 : 0;
    return result;
}

bool VecBool4f::All() const
{
    return b[0] && b[1] && b[2] && b[3];
}

bool VecBool4f::None() const
{
    return !b[0] && !b[1] && !b[2] && !b[3];
}

bool VecBool4f::Any() const
{
    return b[0] || b[1] || b[2] || b[3];
}

bool VecBool4f::All3() const
{
    return b[0] && b[1] && b[2];
}

bool VecBool4f::None3() const
{
    return !b[0] && !b[1] && !b[2];
}

bool VecBool4f::Any3() const
{
    return b[0] || b[1] || b[2];
}

const VecBool4f VecBool4f::operator & (const VecBool4f rhs) const
{
    return VecBool4f{ b[0] && rhs.b[0], b[1] && rhs.b[1], b[2] && rhs.b[2], b[3] && rhs.b[3] };
}

const VecBool4f VecBool4f::operator | (const VecBool4f rhs) const
{
    return VecBool4f{ b[0] || rhs.b[0], b[1] || rhs.b[1], b[2] || rhs.b[2], b[3] || rhs.b[3] };
}

const VecBool4f VecBool4f::operator ^ (const VecBool4f rhs) const
{
    return VecBool4f{ b[0] ^ rhs.b[0], b[1] ^ rhs.b[1], b[2] ^ rhs.b[2], b[3] ^ rhs.b[3] };
}

bool VecBool4f::operator == (const VecBool4f rhs) const
{
    return b[0] == rhs.b[0] && b[1] == rhs.b[1] && b[2] == rhs.b[2] && b[3] == rhs.b[3];
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec4f Vec4f::Zero()
{
    return { 0.0f, 0.0f, 0.0f, 0.0f };
}

#ifdef _DEBUG
Vec4f::Vec4f()
    : x(std::numeric_limits<float>::signaling_NaN())
    , y(std::numeric_limits<float>::signaling_NaN())
    , z(std::numeric_limits<float>::signaling_NaN())
    , w(std::numeric_limits<float>::signaling_NaN())
{}
#else
Vec4f::Vec4f() = default;
#endif // _DEBUG

Vec4f::Vec4f(const Vec4f& other)
    : x(other.x), y(other.y), z(other.z), w(other.w)
{}

Vec4f::Vec4f(const float scalar)
    : x(scalar), y(scalar), z(scalar), w(scalar)
{}

Vec4f::Vec4f(const float x, const float y, const float z, const float w)
    : x(x), y(y), z(z), w(w)
{}

Vec4f::Vec4f(const int32 x, const int32 y, const int32 z, const int32 w)
    : i{ x, y, z, w }
{}

Vec4f::Vec4f(const uint32 x, const uint32 y, const uint32 z, const uint32 w)
    : u{ x, y, z, w }
{}

Vec4f::Vec4f(const int32 scalar)
    : i{ scalar, scalar, scalar, scalar }
{}

Vec4f::Vec4f(const uint32 scalar)
    : u{ scalar, scalar, scalar, scalar }
{}

Vec4f::Vec4f(const float* src)
    : x(src[0])
    , y(src[1])
    , z(src[2])
    , w(src[3])
{}

Vec4f::Vec4f(const Vec2f& src)
    : x(src.x)
    , y(src.y)
    , z(0.0f)
    , w(0.0f)
{}

Vec4f::Vec4f(const Vec3f& src)
    : x(src.x)
    , y(src.y)
    , z(src.z)
    , w(0.0f)
{}

Vec4f::Vec4f(const Vec4fU& src)
    : x(src.x)
    , y(src.y)
    , z(src.z)
    , w(src.w)
{}

Vec4f& Vec4f::operator = (const Vec4f& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
}

const Vec4f Vec4f::FromInteger(int32 x)
{
    return Vec4f{ static_cast<float>(x) };
}

const Vec4f Vec4f::FromIntegers(int32 x, int32 y, int32 z, int32 w)
{
    return Vec4f{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w) };
}

uint32 Vec4f::ToBGR() const
{
    const Vec4f scaled = (*this) * VECTOR_255;

    uint32 ret;
    ret  = Math::Clamp(static_cast<int32>(scaled.z), 0, 255);
    ret |= Math::Clamp(static_cast<int32>(scaled.y), 0, 255) << 8;
    ret |= Math::Clamp(static_cast<int32>(scaled.x), 0, 255) << 16;
    return ret;
}

uint32 Vec4f::ToRGBA() const
{
    const Vec4f scaled = (*this) * VECTOR_255;

    uint32 ret;
    ret  = Math::Clamp(static_cast<int32>(scaled.x), 0, 255);
    ret |= Math::Clamp(static_cast<int32>(scaled.y), 0, 255) << 8;
    ret |= Math::Clamp(static_cast<int32>(scaled.z), 0, 255) << 16;
    ret |= Math::Clamp(static_cast<int32>(scaled.w), 0, 255) << 24;
    return ret;
}

template<uint32 flipX, uint32 flipY, uint32 flipZ, uint32 flipW>
const Vec4f Vec4f::ChangeSign() const
{
    if (!(flipX || flipY || flipZ || flipW))
    {
        // no operation
        return *this;
    }

    return Vec4f
    {
        flipX > 0 ? -x : x,
        flipY > 0 ? -y : y,
        flipZ > 0 ? -z : z,
        flipW > 0 ? -w : w,
    };
}

const Vec4f Vec4f::ChangeSign(const VecBool4f& flip) const
{
    return Vec4f
    {
        flip.Get<0>() ? -x : x,
        flip.Get<1>() ? -y : y,
        flip.Get<2>() ? -z : z,
        flip.Get<3>() ? -w : w,
    };
}

template<uint32 maskX, uint32 maskY, uint32 maskZ, uint32 maskW>
NFE_FORCE_INLINE const Vec4f Vec4f::MakeMask()
{
    static_assert(!(maskX == 0 && maskY == 0 && maskZ == 0 && maskW == 0), "Useless mask");
    static_assert(!(maskX && maskY && maskZ && maskW), "Useless mask");

    // generate bit negation mask
    return Vec4f{ maskX ? 0xFFFFFFFF : 0, maskY ? 0xFFFFFFFF : 0, maskZ ? 0xFFFFFFFF : 0, maskW ? 0xFFFFFFFF : 0 };
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec4f Vec4f::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return { f[ix], f[iy], f[iz], f[iw] };
}

const Vec4f Vec4f::Swizzle(uint32 ix, uint32 iy, uint32 iz, uint32 iw) const
{
    return Vec4f{ f[ix], f[iy], f[iz], f[iw] };
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec4f Vec4f::Shuffle(const Vec4f& a, const Vec4f& b)
{
    return Vec4f{ a[ix], a[iy], b[iz], b[iw] };
}

const Vec4f Vec4f::Select(const Vec4f& a, const Vec4f& b, const VecBool4f& sel)
{
    return Vec4f
    {
        sel.Get<0>() ? b.x : a.x,
        sel.Get<1>() ? b.y : a.y,
        sel.Get<2>() ? b.z : a.z,
        sel.Get<3>() ? b.w : a.w,
    };
}

template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
const Vec4f Vec4f::Select(const Vec4f& a, const Vec4f& b)
{
    static_assert(selX <= 1, "Invalid X index");
    static_assert(selY <= 1, "Invalid Y index");
    static_assert(selZ <= 1, "Invalid Z index");
    static_assert(selW <= 1, "Invalid W index");

    return Vec4f
    {
        selX ? b.x : a.x,
        selY ? b.y : a.y,
        selZ ? b.z : a.z,
        selW ? b.w : a.w,
    };
}

const Vec4f Vec4f::operator & (const Vec4f& b) const
{
    return
    {
        i[0] & b.i[0],
        i[1] & b.i[1],
        i[2] & b.i[2],
        i[3] & b.i[3]
    };
}

const Vec4f Vec4f::operator | (const Vec4f& b) const
{
    return
    {
        i[0] | b.i[0],
        i[1] | b.i[1],
        i[2] | b.i[2],
        i[3] | b.i[3]
    };
}

const Vec4f Vec4f::operator ^ (const Vec4f& b) const
{
    return
    {
        i[0] ^ b.i[0],
        i[1] ^ b.i[1],
        i[2] ^ b.i[2],
        i[3] ^ b.i[3]
    };
}

Vec4f& Vec4f::operator &= (const Vec4f& b)
{
    i[0] &= b.i[0];
    i[1] &= b.i[1];
    i[2] &= b.i[2];
    i[3] &= b.i[3];
    return *this;
}

Vec4f& Vec4f::operator |= (const Vec4f& b)
{
    i[0] |= b.i[0];
    i[1] |= b.i[1];
    i[2] |= b.i[2];
    i[3] |= b.i[3];
    return *this;
}

Vec4f& Vec4f::operator ^= (const Vec4f& b)
{
    i[0] ^= b.i[0];
    i[1] ^= b.i[1];
    i[2] ^= b.i[2];
    i[3] ^= b.i[3];
    return *this;
}

const Vec4f Vec4f::operator - () const
{
    return Vec4f{ -x, -y, -z, -w };
}

const Vec4f Vec4f::operator + (const Vec4f& b) const
{
    return Vec4f{ x + b.x, y + b.y, z + b.z, w + b.w };
}

const Vec4f Vec4f::operator - (const Vec4f& b) const
{
    return Vec4f{ x - b.x, y - b.y, z - b.z, w - b.w };
}

const Vec4f Vec4f::operator * (const Vec4f& b) const
{
    return Vec4f{ x * b.x, y * b.y, z * b.z, w * b.w };
}

const Vec4f Vec4f::operator / (const Vec4f& b) const
{
    return Vec4f{ x / b.x, y / b.y, z / b.z, w / b.w };
}

const Vec4f Vec4f::operator * (float b) const
{
    return Vec4f{ x * b, y * b, z * b, w * b };
}

const Vec4f Vec4f::operator / (float b) const
{
    return Vec4f{ x / b, y / b, z / b, w / b };
}

const Vec4f operator * (float a, const Vec4f& b)
{
    return Vec4f{ a * b.x, a * b.y, a * b.z, a * b.w };
}


Vec4f& Vec4f::operator += (const Vec4f& b)
{
    x += b.x;
    y += b.y;
    z += b.z;
    w += b.w;
    return *this;
}

Vec4f& Vec4f::operator -= (const Vec4f& b)
{
    x -= b.x;
    y -= b.y;
    z -= b.z;
    w -= b.w;
    return *this;
}

Vec4f& Vec4f::operator *= (const Vec4f& b)
{
    x *= b.x;
    y *= b.y;
    z *= b.z;
    w *= b.w;
    return *this;
}

Vec4f& Vec4f::operator /= (const Vec4f& b)
{
    x /= b.x;
    y /= b.y;
    z /= b.z;
    w /= b.w;
    return *this;
}

Vec4f& Vec4f::operator *= (float b)
{
    x *= b;
    y *= b;
    z *= b;
    w *= b;
    return *this;
}

Vec4f& Vec4f::operator /= (float b)
{
    x /= b;
    y /= b;
    z /= b;
    w /= b;
    return *this;
}

const Vec4f Vec4f::Mod1(const Vec4f& x)
{
    return x - Vec4f::Floor(x);
}

const Vec4f Vec4f::MulAndAdd(const Vec4f& a, const Vec4f& b, const Vec4f& c)
{
    return a * b + c;
}

const Vec4f Vec4f::MulAndSub(const Vec4f& a, const Vec4f& b, const Vec4f& c)
{
    return a * b - c;
}

const Vec4f Vec4f::NegMulAndAdd(const Vec4f& a, const Vec4f& b, const Vec4f& c)
{
    return -(a * b) + c;
}

const Vec4f Vec4f::NegMulAndSub(const Vec4f& a, const Vec4f& b, const Vec4f& c)
{
    return -(a * b) - c;
}

const Vec4f Vec4f::Floor(const Vec4f& v)
{
    return Vec4f{ floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w) };
}

const Vec4f Vec4f::Sqrt(const Vec4f& v)
{
    return Vec4f{ sqrtf(v.x), sqrtf(v.y), sqrtf(v.z), sqrtf(v.w) };
}

const Vec4f Vec4f::Reciprocal(const Vec4f& v)
{
    return Vec4f{ 1.0f / v.x, 1.0f / v.y, 1.0f / v.z, 1.0f / v.w };
}

const Vec4f Vec4f::FastReciprocal(const Vec4f& v)
{
    return Reciprocal(v); // TODO
}

const Vec4f Vec4f::Min(const Vec4f& a, const Vec4f& b)
{
    return Vec4f
    {
        Math::Min(a.x, b.x),
        Math::Min(a.y, b.y),
        Math::Min(a.z, b.z),
        Math::Min(a.w, b.w)
    };
}

const Vec4f Vec4f::Max(const Vec4f& a, const Vec4f& b)
{
    return Vec4f
    {
        Math::Max(a.x, b.x),
        Math::Max(a.y, b.y),
        Math::Max(a.z, b.z),
        Math::Max(a.w, b.w)
    };
}

const Vec4f Vec4f::Abs(const Vec4f& v)
{
    return Vec4f
    {
        fabsf(v.x),
        fabsf(v.y),
        fabsf(v.z),
        fabsf(v.w)
    };
}

uint32 Vec4f::GetSignMask() const
{
    uint32 ret = 0;
    ret |= x < 0.0f ? (1u << 0u) : 0u;
    ret |= y < 0.0f ? (1u << 1u) : 0u;
    ret |= z < 0.0f ? (1u << 2u) : 0u;
    ret |= w < 0.0f ? (1u << 3u) : 0u;
    return ret;
}

const Vec4f Vec4f::HorizontalMax() const
{
    return Vec4f{ Math::Max(Math::Max(x, y), Math::Max(z, w)) };
}

const Vec4f Vec4f::HorizontalSum() const
{
    return Vec4f{ x + y + z + w };
}

const VecBool4f Vec4f::operator == (const Vec4f& b) const
{
    return VecBool4f{ x == b.x, y == b.y, z == b.z, w == b.w };
}

const VecBool4f Vec4f::operator != (const Vec4f& b) const
{
    return VecBool4f{ x != b.x, y != b.y, z != b.z, w != b.w };
}

const VecBool4f Vec4f::operator < (const Vec4f& b) const
{
    return VecBool4f{ x < b.x, y < b.y, z < b.z, w < b.w };
}

const VecBool4f Vec4f::operator <= (const Vec4f& b) const
{
    return VecBool4f{ x <= b.x, y <= b.y, z <= b.z, w <= b.w };
}

const VecBool4f Vec4f::operator > (const Vec4f& b) const
{
    return VecBool4f{ x > b.x, y > b.y, z > b.z, w > b.w };
}

const VecBool4f Vec4f::operator >= (const Vec4f& b) const
{
    return VecBool4f{ x >= b.x, y >= b.y, z >= b.z, w >= b.w };
}

float Vec4f::Dot2(const Vec4f& a, const Vec4f& b)
{
    return a.x * b.x + a.y * b.y;
}

float Vec4f::Dot3(const Vec4f& a, const Vec4f& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Vec4f::Dot4(const Vec4f& a, const Vec4f& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

const Vec4f Vec4f::Dot2V(const Vec4f& a, const Vec4f& b)
{
    return Vec4f{ Dot2(a, b) };
}

const Vec4f Vec4f::Dot3V(const Vec4f& a, const Vec4f& b)
{
    return Vec4f{ Dot3(a, b) };
}

const Vec4f Vec4f::Dot4V(const Vec4f& a, const Vec4f& b)
{
    return Vec4f{ Dot4(a, b) };
}

const Vec4f Vec4f::Cross3(const Vec4f& v1, const Vec4f& v2)
{
    return Vec4f
    {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x,
    };
}

float Vec4f::Length2() const
{
    return sqrtf(SqrLength2());
}

const Vec4f Vec4f::Length2V() const
{
    return Vec4f{ Length2() };
}

float Vec4f::Length3() const
{
    return sqrtf(Dot3(*this, *this));
}

float Vec4f::SqrLength3() const
{
    return Dot3(*this, *this);
}

const Vec4f Vec4f::Length3V() const
{
    return Vec4f{ Length3() };
}

Vec4f& Vec4f::Normalize3()
{
    *this /= Length3V();
    return *this;
}

Vec4f& Vec4f::FastNormalize3()
{
    // TODO
    *this /= Length3V();
    return *this;
}

float Vec4f::Length4() const
{
    return sqrtf(SqrLength4());
}

const Vec4f Vec4f::Length4V() const
{
    return Vec4f{ Length4() };
}

Vec4f& Vec4f::Normalize4()
{
    *this /= Length4V();
    return *this;
}

const Vec4f Vec4f::Fmod1(const Vec4f& v)
{
    return Vec4f{ fmodf(v.x, 1.0f), fmodf(v.y, 1.0f), fmodf(v.z, 1.0f), fmodf(v.w, 1.0f) };
}

const VecBool4f Vec4f::IsNaN() const
{
    return VecBool4f{ Math::IsNaN(x), Math::IsNaN(y), Math::IsNaN(z), Math::IsNaN(w) };
}

const VecBool4f Vec4f::IsInfinite() const
{
    return VecBool4f{ Math::IsInfinity(x), Math::IsInfinity(y), Math::IsInfinity(z), Math::IsInfinity(w) };
}

bool Vec4f::IsValid() const
{
    return Math::IsValid(x) && Math::IsValid(y) && Math::IsValid(z) && Math::IsValid(w);
}

void Vec4f::Transpose3(Vec4f& a, Vec4f& b, Vec4f& c)
{
    std::swap(a.y, b.x);
    std::swap(a.z, c.x);
    std::swap(b.z, c.y);
}

} // namespace Math
} // namespace NFE
