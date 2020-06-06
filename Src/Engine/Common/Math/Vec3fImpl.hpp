#pragma once

#include "Vec3f.hpp"


namespace NFE {
namespace Math {


static_assert(sizeof(Vec3f) == 3 * sizeof(float), "Invalid Vec3f size");


constexpr Vec3f::Vec3f()
    : x(0.0f), y(0.0f), z(0.0f)
{ }

constexpr Vec3f::Vec3f(const float* src)
    : x(src[0]), y(src[1]), z(src[2])
{ }

constexpr Vec3f::Vec3f(const Vec2f& src, const float z)
    : x(src.x), y(src.y), z(z)
{ }

constexpr Vec3f::Vec3f(float s)
    : x(s), y(s), z(s)
{ }

constexpr Vec3f::Vec3f(float x, float y, float z)
    : x(x), y(y), z(z)
{ }

float Vec3f::Get(uint32 index) const
{
    NFE_ASSERT(index < 3, "Invalid index");
    return (&x)[index];
}

float& Vec3f::Get(uint32 index)
{
    NFE_ASSERT(index < 3, "Invalid index");
    return (&x)[index];
}

Vec3f::operator Vec2f() const
{
    return Vec2f(x, y);
}

bool Vec3f::IsValid() const
{
    return Math::IsValid(x) && Math::IsValid(y) && Math::IsValid(z);
}

//////////////////////////////////////////////////////////////////////////

template<uint32 ix, uint32 iy, uint32 iz>
const Vec3f Vec3f::Swizzle() const
{
    static_assert(ix < 3, "Invalid X element index");
    static_assert(iy < 3, "Invalid Y element index");
    static_assert(iz < 3, "Invalid Z element index");

    return Vec3f((&x)[ix], (&x)[iy], (&x)[iz]);
}

template<uint32 ix, uint32 iy, uint32 iz>
constexpr const Vec3f Vec3f::Blend(const Vec3f& a, const Vec3f& b)
{
    return Vec3f(ix == 0 ? a.x : b.x,
                  iy == 0 ? a.y : b.y,
                  iz == 0 ? a.z : b.z);
}

template<bool changeX, bool changeY, bool changeZ>
constexpr const Vec3f Vec3f::ChangeSign() const
{
    return Vec3f(
        changeX ? -x : x,
        changeY ? -y : y,
        changeZ ? -z : z
    );
}

const Vec3f Vec3f::SelectBySign(const Vec3f& a, const Vec3f& b, const Vec3f& sel)
{
    Vec3f ret;
    ret.x = sel.x > 0.0f ? a.x : b.x;
    ret.y = sel.y > 0.0f ? a.y : b.y;
    ret.z = sel.z > 0.0f ? a.z : b.z;
    return ret;
}

constexpr const Vec3f Vec3f::SplatX() const
{
    return Vec3f(x, x, x);
}

constexpr const Vec3f Vec3f::SplatY() const
{
    return Vec3f(y, y, y);
}

constexpr const Vec3f Vec3f::SplatZ() const
{
    return Vec3f(z, z, z);
}

constexpr const Vec3f Vec3f::Splat(float f)
{
    return Vec3f(f, f, f);
}


//////////////////////////////////////////////////////////////////////////

constexpr const Vec3f Vec3f::operator- () const
{
    return Vec3f(-x, -y, -z);
}

constexpr const Vec3f Vec3f::operator+ (const Vec3f& b) const
{
    return Vec3f(x + b.x, y + b.y, z + b.z);
}

constexpr const Vec3f Vec3f::operator- (const Vec3f& b) const
{
    return Vec3f(x - b.x, y - b.y, z - b.z);
}

constexpr const Vec3f Vec3f::operator* (const Vec3f& b) const
{
    return Vec3f(x * b.x, y * b.y, z * b.z);
}

const Vec3f Vec3f::operator/ (const Vec3f& b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b.x) > FLT_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.y) > FLT_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.z) > FLT_EPSILON, "Division by zero");

    return Vec3f(x / b.x, y / b.y, z / b.z);
}

constexpr const Vec3f Vec3f::operator* (float b) const
{
    return Vec3f(x * b, y * b, z * b);
}

const Vec3f Vec3f::operator/ (float b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b) > FLT_EPSILON, "Division by zero");

    return Vec3f(x / b, y / b, z / b);
}

const Vec3f operator*(float a, const Vec3f& b)
{
    return Vec3f(a * b.x, a * b.y, a * b.z);
}

//////////////////////////////////////////////////////////////////////////

Vec3f& Vec3f::operator+= (const Vec3f& b)
{
    x += b.x;
    y += b.y;
    z += b.z;
    return *this;
}

Vec3f& Vec3f::operator-= (const Vec3f& b)
{
    x -= b.x;
    y -= b.y;
    z -= b.z;
    return *this;
}

Vec3f& Vec3f::operator*= (const Vec3f& b)
{
    x *= b.x;
    y *= b.y;
    z *= b.z;
    return *this;
}

Vec3f& Vec3f::operator/= (const Vec3f& b)
{
    NFE_ASSERT(Math::Abs(b.x) > FLT_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.y) > FLT_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.z) > FLT_EPSILON, "Division by zero");

    x /= b.x;
    y /= b.y;
    z /= b.z;
    return *this;
}

Vec3f& Vec3f::operator*= (float b)
{
    x *= b;
    y *= b;
    z *= b;
    return *this;
}

Vec3f& Vec3f::operator/= (float b)
{
    NFE_ASSERT(Math::Abs(b) > FLT_EPSILON, "Division by zero");

    x /= b;
    y /= b;
    z /= b;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const Vec3f Vec3f::Floor(const Vec3f& v)
{
    return Vec3f(floorf(v.x), floorf(v.y), floorf(v.z));
}

const Vec3f Vec3f::Sqrt(const Vec3f& v)
{
    NFE_ASSERT(v.x >= 0.0f, "Sqrt of negative");
    NFE_ASSERT(v.y >= 0.0f, "Sqrt of negative");
    NFE_ASSERT(v.z >= 0.0f, "Sqrt of negative");

    return Vec3f(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z));
}

const Vec3f Vec3f::Reciprocal(const Vec3f& v)
{
    NFE_ASSERT(Math::Abs(v.x) > FLT_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.y) > FLT_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.z) > FLT_EPSILON, "Division by zero");

    // this checks are required to avoid "potential divide by 0" warning
    return Vec3f(v.x != 0.0f ? 1.0f / v.x : INFINITY,
                  v.y != 0.0f ? 1.0f / v.y : INFINITY,
                  v.z != 0.0f ? 1.0f / v.z : INFINITY);
}

constexpr const Vec3f Vec3f::Min(const Vec3f& a, const Vec3f& b)
{
    return Vec3f(
        Math::Min<float>(a.x, b.x),
        Math::Min<float>(a.y, b.y),
        Math::Min<float>(a.z, b.z)
    );
}

constexpr const Vec3f Vec3f::Max(const Vec3f& a, const Vec3f& b)
{
    return Vec3f(
        Math::Max<float>(a.x, b.x),
        Math::Max<float>(a.y, b.y),
        Math::Max<float>(a.z, b.z)
    );
}

constexpr const Vec3f Vec3f::Abs(const Vec3f& v)
{
    return Vec3f(Math::Abs(v.x), Math::Abs(v.y), Math::Abs(v.z));
}

constexpr const Vec3f Vec3f::Lerp(const Vec3f& v1, const Vec3f& v2, const Vec3f& weight)
{
    return Vec3f(
        v1.x + weight.x * (v2.x - v1.x),
        v1.y + weight.y * (v2.y - v1.y),
        v1.z + weight.z * (v2.z - v1.z)
    );
}

constexpr const Vec3f Vec3f::Lerp(const Vec3f& v1, const Vec3f& v2, float weight)
{
    return Vec3f(
        v1.x + weight * (v2.x - v1.x),
        v1.y + weight * (v2.y - v1.y),
        v1.z + weight * (v2.z - v1.z)
    );
}

//////////////////////////////////////////////////////////////////////////

constexpr bool Vec3f::operator== (const Vec3f& b) const
{
    return (x == b.x) && (y == b.y) && (z == b.z);
}

constexpr bool Vec3f::operator< (const Vec3f& b) const
{
    return (x < b.x) && (y < b.y) && (z < b.z);
}

constexpr bool Vec3f::operator<= (const Vec3f& b) const
{
    return (x <= b.x) && (y <= b.y) && (z <= b.z);
}

constexpr bool Vec3f::operator> (const Vec3f& b) const
{
    return (x > b.x) && (y > b.y) && (z > b.z);
}

constexpr bool Vec3f::operator>= (const Vec3f& b) const
{
    return (x >= b.x) && (y >= b.y) && (z >= b.z);
}

constexpr bool Vec3f::operator!= (const Vec3f& b) const
{
    return (x != b.x) && (y != b.y) && (z != b.z);
}

constexpr bool Vec3f::AlmostEqual(const Vec3f& v1, const Vec3f& v2, float epsilon)
{
    return Abs(v1 - v2) < Vec3f::Splat(epsilon);
}

//////////////////////////////////////////////////////////////////////////

constexpr float Vec3f::Dot(const Vec3f& a, const Vec3f& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr const Vec3f Vec3f::Cross(const Vec3f& a, const Vec3f& b)
{
    return Vec3f(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

float Vec3f::Length() const
{
    return sqrtf(x * x + y * y + z * z);
}

const Vec3f Vec3f::Normalized() const
{
    const float len = Length();
    NFE_ASSERT(len > FLT_EPSILON, "Normalizing near-zero-length vector");

    const float lenInv = 1.0f / len;
    return *this * lenInv;
}

Vec3f& Vec3f::Normalize()
{
    const float len = Length();
    NFE_ASSERT(len > FLT_EPSILON, "Normalizing near-zero-length vector");

    const float lenInv = 1.0f / len;
    *this *= lenInv;
    return *this;
}


} // namespace Math
} // namespace NFE
