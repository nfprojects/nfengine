#pragma once

#include "Vec4f.hpp"

namespace NFE {
namespace Math {

VecBool4i::VecBool4i(bool scalar)
    : b{ scalar, scalar, scalar, scalar }
{}

VecBool4i::VecBool4i(bool x, bool y, bool z, bool w)
    : b{ x, y, z, w }
{}

VecBool4i::VecBool4i(int32 x, int32 y, int32 z, int32 w)
    : b{ x > 0, y > 0, z > 0, w > 0 }
{}

VecBool4i::VecBool4i(const VecBool4f& other)
    : b{ other.Get<0>(), other.Get<1>(), other.Get<2>(), other.Get<3>() }
{}

template<uint32 index>
bool VecBool4i::Get() const
{
    static_assert(index < 4, "Invalid index");
    return b[index];
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const VecBool4i VecBool4i::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return VecBool4i{ b[ix], b[iy], b[iz], b[iw] };
}

// combine into 4-bit mask
int32 VecBool4i::GetMask() const
{
    int32 ret = 0;
    ret |= b[0] ? (1 << 0) : 0;
    ret |= b[1] ? (1 << 1) : 0;
    ret |= b[2] ? (1 << 2) : 0;
    ret |= b[3] ? (1 << 3) : 0;
    return ret;
}

bool VecBool4i::All() const
{
    return b[0] && b[1] && b[2] && b[3];
}

bool VecBool4i::None() const
{
    return (!b[0]) && (!b[1]) && (!b[2]) && (!b[3]);
}

bool VecBool4i::Any() const
{
    return b[0] || b[1] || b[2] || b[3];
}

bool VecBool4i::All3() const
{
    return b[0] && b[1] && b[2];
}

bool VecBool4i::None3() const
{
    return (!b[0]) && (!b[1]) && (!b[2]);
}

bool VecBool4i::Any3() const
{
    return b[0] || b[1] || b[2];
}

const VecBool4i VecBool4i::operator & (const VecBool4i rhs) const
{
    return VecBool4i{ b[0] && rhs.b[0], b[1] && rhs.b[1], b[2] && rhs.b[2], b[3] && rhs.b[3] };
}

const VecBool4i VecBool4i::operator | (const VecBool4i rhs) const
{
    return VecBool4i{ b[0] || rhs.b[0], b[1] || rhs.b[1], b[2] || rhs.b[2], b[3] || rhs.b[3] };
}

const VecBool4i VecBool4i::operator ^ (const VecBool4i rhs) const
{
    return VecBool4i{ b[0] ^ rhs.b[0], b[1] ^ rhs.b[1], b[2] ^ rhs.b[2], b[3] ^ rhs.b[3] };
}

bool VecBool4i::operator == (const VecBool4i& rhs) const
{
    return (b[0] == rhs.b[0]) && (b[1] == rhs.b[1]) && (b[2] == rhs.b[2]) && (b[3] == rhs.b[3]);
}

bool VecBool4i::operator != (const VecBool4i& rhs) const
{
    return (b[0] != rhs.b[0]) || (b[1] != rhs.b[1]) || (b[2] != rhs.b[2]) || (b[3] != rhs.b[3]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec4i Vec4i::Zero()
{
    return Vec4i{ 0, 0, 0, 0 };
}

Vec4i::Vec4i(const Vec4i& other)
    : x(other.x), y(other.y), z(other.z), w(other.w)
{}

Vec4i& Vec4i::operator = (const Vec4i& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
}

Vec4i::Vec4i(const Vec4ui& other)
    : x((int32)other.x), y((int32)other.y), z((int32)other.z), w((int32)other.w)
{}

Vec4i::Vec4i(const VecBool4i& other)
    : x(other.b[0] ? 0xFFFFFFFF : 0)
    , y(other.b[1] ? 0xFFFFFFFF : 0)
    , z(other.b[2] ? 0xFFFFFFFF : 0)
    , w(other.b[3] ? 0xFFFFFFFF : 0)
{}

const Vec4i Vec4i::Cast(const Vec4f& v)
{
    return reinterpret_cast<const Vec4i&>(v);
}

const Vec4f Vec4i::AsVec4f() const
{
    return reinterpret_cast<const Vec4f&>(*this);
}

Vec4i::Vec4i(const int32 x, const int32 y, const int32 z, const int32 w)
    : i{ x, y, z, w }
{}

Vec4i::Vec4i(const int32 scalar)
    : i{ scalar, scalar, scalar, scalar }
{}

Vec4i::Vec4i(const int32* scalarPtr)
    : i{ scalarPtr[0], scalarPtr[1], scalarPtr[2], scalarPtr[3] }
{}

const Vec4i Vec4i::Convert(const Vec4f& v)
{
    return
    {
        static_cast<int32>(v.x),
        static_cast<int32>(v.y),
        static_cast<int32>(v.z),
        static_cast<int32>(v.w)
    };
}

const Vec4i Vec4i::TruncateAndConvert(const Vec4f& v)
{
    // TODO
    return
    {
        static_cast<int32>(v.x),
        static_cast<int32>(v.y),
        static_cast<int32>(v.z),
        static_cast<int32>(v.w)
    };
}

const Vec4f Vec4i::ConvertToVec4f() const
{
    return Vec4f
    {
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        static_cast<float>(w)
    };
}

const Vec4i Vec4i::Select(const Vec4i& a, const Vec4i& b, const VecBool4i& sel)
{
    return
    {
        sel.Get<0>() ? b.x : a.x,
        sel.Get<1>() ? b.y : a.y,
        sel.Get<2>() ? b.z : a.z,
        sel.Get<3>() ? b.w : a.w,
    };
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec4i Vec4i::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return { i[ix], i[iy], i[iz], i[iw] };
}

const Vec4i Vec4i::operator & (const Vec4i& b) const
{
    return { x & b.x, y & b.y, z & b.z, w & b.w };
}

const Vec4i Vec4i::AndNot(const Vec4i& a, const Vec4i& b)
{
    return { (~a.x) & b.x, (~a.y) & b.y, (~a.z) & b.z, (~a.w) & b.w };
}

const Vec4i Vec4i::operator | (const Vec4i& b) const
{
    return { x | b.x, y | b.y, z | b.z, w | b.w };
}

const Vec4i Vec4i::operator ^ (const Vec4i& b) const
{
    return { x ^ b.x, y ^ b.y, z ^ b.z, w ^ b.w };
}

Vec4i& Vec4i::operator &= (const Vec4i& b)
{
    x &= b.x;
    y &= b.y;
    z &= b.z;
    w &= b.w;
    return *this;
}

Vec4i& Vec4i::operator |= (const Vec4i& b)
{
    x |= b.x;
    y |= b.y;
    z |= b.z;
    w |= b.w;
    return *this;
}

Vec4i& Vec4i::operator ^= (const Vec4i& b)
{
    x ^= b.x;
    y ^= b.y;
    z ^= b.z;
    w ^= b.w;
    return *this;
}

const Vec4i Vec4i::operator - () const
{
    return Vec4i::Zero() - (*this);
}

const Vec4i Vec4i::operator + (const Vec4i& b) const
{
    return { x + b.x, y + b.y, z + b.z, w + b.w };
}

const Vec4i Vec4i::operator - (const Vec4i& b) const
{
    return { x - b.x, y - b.y, z - b.z, w - b.w };
}

const Vec4i Vec4i::operator * (const Vec4i& b) const
{
    return { x * b.x, y * b.y, z * b.z, w * b.w };
}

Vec4i& Vec4i::operator += (const Vec4i& b)
{
    x += b.x;
    y += b.y;
    z += b.z;
    w += b.w;
    return *this;
}

Vec4i& Vec4i::operator -= (const Vec4i& b)
{
    x -= b.x;
    y -= b.y;
    z -= b.z;
    w -= b.w;
    return *this;
}

Vec4i& Vec4i::operator *= (const Vec4i& b)
{
    x *= b.x;
    y *= b.y;
    z *= b.z;
    w *= b.w;
    return *this;
}

const Vec4i Vec4i::operator + (int32 b) const
{
    return { x + b, y + b, z + b, w + b };
}

const Vec4i Vec4i::operator - (int32 b) const
{
    return { x - b, y - b, z - b, w - b };
}

const Vec4i Vec4i::operator * (int32 b) const
{
    return { x * b, y * b, z * b, w * b };
}

Vec4i& Vec4i::operator += (int32 b)
{
    x += b;
    y += b;
    z += b;
    w += b;
    return *this;
}

Vec4i& Vec4i::operator -= (int32 b)
{
    x -= b;
    y -= b;
    z -= b;
    w -= b;
    return *this;
}

Vec4i& Vec4i::operator *= (int32 b)
{
    x *= b;
    y *= b;
    z *= b;
    w *= b;
    return *this;
}

const Vec4i Vec4i::operator << (const Vec4i& b) const
{
    return { x << b.x, y << b.y, z << b.z, w << b.w };
}

const Vec4i Vec4i::operator >> (const Vec4i& b) const
{
    return { x >> b.x, y >> b.y, z >> b.z, w >> b.w };
}

Vec4i& Vec4i::operator <<= (const Vec4i& b)
{
    x <<= b.x;
    y <<= b.y;
    z <<= b.z;
    w <<= b.w;
    return *this;
}

Vec4i& Vec4i::operator >>= (const Vec4i& b)
{
    x >>= b.x;
    y >>= b.y;
    z >>= b.z;
    w >>= b.w;
    return *this;
}

const Vec4i Vec4i::operator << (int32 b) const
{
    return { x << b, y << b, z << b, w << b };
}

const Vec4i Vec4i::operator >> (int32 b) const
{
    return { x >> b, y >> b, z >> b, w >> b };
}

Vec4i& Vec4i::operator <<= (int32 b)
{
    x <<= b;
    y <<= b;
    z <<= b;
    w <<= b;
    return *this;
}

Vec4i& Vec4i::operator >>= (int32 b)
{
    x >>= b;
    y >>= b;
    z >>= b;
    w >>= b;
    return *this;
}

const VecBool4i Vec4i::operator == (const Vec4i& b) const
{
    return VecBool4i{ x == b.x, y == b.y, z == b.z, w == b.w };
}

const VecBool4i Vec4i::operator != (const Vec4i& b) const
{
    return VecBool4i{ x != b.x, y != b.y, z != b.z, w != b.w };
}

const VecBool4i Vec4i::operator < (const Vec4i& b) const
{
    return VecBool4i{ x < b.x, y < b.y, z < b.z, w < b.w };
}

const VecBool4i Vec4i::operator > (const Vec4i& b) const
{
    return VecBool4i{ x > b.x, y > b.y, z > b.z, w > b.w };
}

const VecBool4i Vec4i::operator >= (const Vec4i& b) const
{
    return VecBool4i{ x >= b.x, y >= b.y, z >= b.z, w >= b.w };
}

const VecBool4i Vec4i::operator <= (const Vec4i& b) const
{
    return VecBool4i{ x <= b.x, y <= b.y, z <= b.z, w <= b.w };
}

const Vec4i Vec4i::Min(const Vec4i& a, const Vec4i& b)
{
    return
    {
        Math::Min(a.x, b.x),
        Math::Min(a.y, b.y),
        Math::Min(a.z, b.z),
        Math::Min(a.w, b.w)
    };
}

const Vec4i Vec4i::Max(const Vec4i& a, const Vec4i& b)
{
    return
    {
        Math::Max(a.x, b.x),
        Math::Max(a.y, b.y),
        Math::Max(a.z, b.z),
        Math::Max(a.w, b.w)
    };
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec4ui Vec4ui::Zero()
{
    return Vec4ui{ 0u };
}

Vec4ui::Vec4ui(const Vec4ui& other)
    : x(other.x), y(other.y), z(other.z), w(other.w)
{}

Vec4ui::Vec4ui(const Vec4i& other)
    : x((uint32)other.x), y((uint32)other.y), z((uint32)other.z), w((uint32)other.w)
{}

Vec4ui::Vec4ui(const VecBool4i& other)
    : x(other.b[0] ? 0xFFFFFFFF : 0)
    , y(other.b[1] ? 0xFFFFFFFF : 0)
    , z(other.b[2] ? 0xFFFFFFFF : 0)
    , w(other.b[3] ? 0xFFFFFFFF : 0)
{}

const Vec4ui Vec4ui::Cast(const Vec4f& v)
{
    return reinterpret_cast<const Vec4ui&>(v);
}

const Vec4f Vec4ui::AsVec4f() const
{
    return reinterpret_cast<const Vec4f&>(*this);
}

Vec4ui::Vec4ui(const uint32 x, const uint32 y, const uint32 z, const uint32 w)
    : u{ x, y, z, w }
{}

Vec4ui::Vec4ui(const uint32 scalar)
    : u{ scalar, scalar, scalar, scalar }
{}

Vec4ui::Vec4ui(const uint32* scalarPtr)
    : u{ scalarPtr[0], scalarPtr[1], scalarPtr[2], scalarPtr[3] }
{}

Vec4ui& Vec4ui::operator = (const Vec4ui& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
}

const Vec4ui Vec4ui::Convert(const Vec4f& v)
{
    return
    {
        static_cast<uint32>(v.x),
        static_cast<uint32>(v.y),
        static_cast<uint32>(v.z),
        static_cast<uint32>(v.w)
    };
}

const Vec4ui Vec4ui::TruncateAndConvert(const Vec4f& v)
{
    // TODO
    return
    {
        static_cast<uint32>(v.x),
        static_cast<uint32>(v.y),
        static_cast<uint32>(v.z),
        static_cast<uint32>(v.w)
    };
}

const Vec4f Vec4ui::ConvertToVec4f() const
{
    return Vec4f
    {
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        static_cast<float>(w)
    };
}

//////////////////////////////////////////////////////////////////////////

const Vec4ui Vec4ui::Select(const Vec4ui& a, const Vec4ui& b, const VecBool4i& sel)
{
    return
    {
        sel.Get<0>() ? b.x : a.x,
        sel.Get<1>() ? b.y : a.y,
        sel.Get<2>() ? b.z : a.z,
        sel.Get<3>() ? b.w : a.w,
    };
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec4ui Vec4ui::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return { u[ix], u[iy], u[iz], u[iw] };
}

//////////////////////////////////////////////////////////////////////////

const Vec4ui Vec4ui::operator & (const Vec4ui& b) const
{
    return { x & b.x, y & b.y, z & b.z, w & b.w };
}

const Vec4ui Vec4ui::AndNot(const Vec4ui& a, const Vec4ui& b)
{
    return { (~a.x) & b.x, (~a.y) & b.y, (~a.z) & b.z, (~a.w) & b.w };
}

const Vec4ui Vec4ui::operator | (const Vec4ui& b) const
{
    return { x | b.x, y | b.y, z | b.z, w | b.w };
}

const Vec4ui Vec4ui::operator ^ (const Vec4ui& b) const
{
    return { x ^ b.x, y ^ b.y, z ^ b.z, w ^ b.w };
}

Vec4ui& Vec4ui::operator &= (const Vec4ui& b)
{
    x &= b.x;
    y &= b.y;
    z &= b.z;
    w &= b.w;
    return *this;
}

Vec4ui& Vec4ui::operator |= (const Vec4ui& b)
{
    x |= b.x;
    y |= b.y;
    z |= b.z;
    w |= b.w;
    return *this;
}

Vec4ui& Vec4ui::operator ^= (const Vec4ui& b)
{
    x ^= b.x;
    y ^= b.y;
    z ^= b.z;
    w ^= b.w;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const Vec4ui Vec4ui::operator - () const
{
    return Vec4ui::Zero() - (*this);
}

const Vec4ui Vec4ui::operator + (const Vec4ui& b) const
{
    return { x + b.x, y + b.y, z + b.z, w + b.w };
}

const Vec4ui Vec4ui::operator - (const Vec4ui& b) const
{
    return { x - b.x, y - b.y, z - b.z, w - b.w };
}

const Vec4ui Vec4ui::operator * (const Vec4ui& b) const
{
    return { x * b.x, y * b.y, z * b.z, w * b.w };
}

Vec4ui& Vec4ui::operator += (const Vec4ui& b)
{
    x += b.x;
    y += b.y;
    z += b.z;
    w += b.w;
    return *this;
}

Vec4ui& Vec4ui::operator -= (const Vec4ui& b)
{
    x -= b.x;
    y -= b.y;
    z -= b.z;
    w -= b.w;
    return *this;
}

Vec4ui& Vec4ui::operator *= (const Vec4ui& b)
{
    x *= b.x;
    y *= b.y;
    z *= b.z;
    w *= b.w;
    return *this;
}

const Vec4ui Vec4ui::operator + (uint32 b) const
{
    return { x + b, y + b, z + b, w + b };
}

const Vec4ui Vec4ui::operator - (uint32 b) const
{
    return { x - b, y - b, z - b, w - b };
}

const Vec4ui Vec4ui::operator * (uint32 b) const
{
    return { x * b, y * b, z * b, w * b };
}

Vec4ui& Vec4ui::operator += (uint32 b)
{
    x += b;
    y += b;
    z += b;
    w += b;
    return *this;
}

Vec4ui& Vec4ui::operator -= (uint32 b)
{
    x -= b;
    y -= b;
    z -= b;
    w -= b;
    return *this;
}

Vec4ui& Vec4ui::operator *= (uint32 b)
{
    x *= b;
    y *= b;
    z *= b;
    w *= b;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const Vec4ui Vec4ui::operator << (const Vec4ui& b) const
{
    return { x << b.x, y << b.y, z << b.z, w << b.w };
}

const Vec4ui Vec4ui::operator >> (const Vec4ui& b) const
{
    return { x >> b.x, y >> b.y, z >> b.z, w >> b.w };
}

Vec4ui& Vec4ui::operator <<= (const Vec4ui& b)
{
    x <<= b.x;
    y <<= b.y;
    z <<= b.z;
    w <<= b.w;
    return *this;
}

Vec4ui& Vec4ui::operator >>= (const Vec4ui& b)
{
    x >>= b.x;
    y >>= b.y;
    z >>= b.z;
    w >>= b.w;
    return *this;
}

const Vec4ui Vec4ui::operator << (uint32 b) const
{
    return { x << b, y << b, z << b, w << b };
}

const Vec4ui Vec4ui::operator >> (uint32 b) const
{
    return { x >> b, y >> b, z >> b, w >> b };
}

Vec4ui& Vec4ui::operator <<= (uint32 b)
{
    x <<= b;
    y <<= b;
    z <<= b;
    w <<= b;
    return *this;
}

Vec4ui& Vec4ui::operator >>= (uint32 b)
{
    x >>= b;
    y >>= b;
    z >>= b;
    w >>= b;
    return *this;
}

const VecBool4i Vec4ui::operator == (const Vec4ui& b) const
{
    return VecBool4i{ x == b.x, y == b.y, z == b.z, w == b.w };
}

const VecBool4i Vec4ui::operator != (const Vec4ui& b) const
{
    return VecBool4i{ x != b.x, y != b.y, z != b.z, w != b.w };
}

const VecBool4i Vec4ui::operator < (const Vec4ui& b) const
{
    return VecBool4i{ x < b.x, y < b.y, z < b.z, w < b.w };
}

const VecBool4i Vec4ui::operator > (const Vec4ui& b) const
{
    return VecBool4i{ x > b.x, y > b.y, z > b.z, w > b.w };
}

const VecBool4i Vec4ui::operator >= (const Vec4ui& b) const
{
    return VecBool4i{ x >= b.x, y >= b.y, z >= b.z, w >= b.w };
}

const VecBool4i Vec4ui::operator <= (const Vec4ui& b) const
{
    return VecBool4i{ x <= b.x, y <= b.y, z <= b.z, w <= b.w };
}

const Vec4ui Vec4ui::Min(const Vec4ui& a, const Vec4ui& b)
{
    return
    {
        Math::Min(a.x, b.x),
        Math::Min(a.y, b.y),
        Math::Min(a.z, b.z),
        Math::Min(a.w, b.w)
    };
}

const Vec4ui Vec4ui::Max(const Vec4ui& a, const Vec4ui& b)
{
    return
    {
        Math::Max(a.x, b.x),
        Math::Max(a.y, b.y),
        Math::Max(a.z, b.z),
        Math::Max(a.w, b.w)
    };
}


} // namespace Math
} // namespace NFE
