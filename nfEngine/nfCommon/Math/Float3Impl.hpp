/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Float3 class definition.
 */

#pragma once

#include "Float3.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace Math {


static_assert(sizeof(Float3) == 3 * sizeof(float), "Invalid Float3 size");


constexpr Float3::Float3()
    : x(0.0f), y(0.0f), z(0.0f)
{ }

constexpr Float3::Float3(const float* src)
    : x(src[0]), y(src[1]), z(src[2])
{ }

constexpr Float3::Float3(const Float2& src)
    : x(src.x), y(src.y), z(0.0f)
{ }

constexpr Float3::Float3(float x, float y, float z)
    : x(x), y(y), z(z)
{ }

float Float3::Get(uint32 index) const
{
    NFE_ASSERT(index < 3, "Invalid vector index");
    return (&x)[index];
}

float& Float3::Get(uint32 index)
{
    NFE_ASSERT(index < 3, "Invalid vector index");
    return (&x)[index];
}

Float3::operator Float2() const
{
    return Float2(x, y);
}

//////////////////////////////////////////////////////////////////////////

template<uint32 ix, uint32 iy, uint32 iz>
Float3 Float3::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");

    return Float3((&x)[ix], (&x)[iy], (&x)[iz]);
}

template<uint32 ix, uint32 iy, uint32 iz>
constexpr Float3 Float3::Blend(const Float3& a, const Float3& b)
{
    return Float3(ix == 0 ? a.x : b.x,
                  iy == 0 ? a.y : b.y,
                  iz == 0 ? a.z : b.z);
}

template<bool negX, bool negY, bool negZ>
constexpr Float3 Float3::ChangeSign() const
{
    return Float3(
        negX ? -x : x,
        negY ? -y : y,
        negZ ? -z : z
    );
}

Float3 Float3::SelectBySign(const Float3& a, const Float3& b, const Float3& sel)
{
    Float3 ret;
    ret.x = sel.x > 0.0f ? a.x : b.x;
    ret.y = sel.y > 0.0f ? a.y : b.y;
    ret.z = sel.z > 0.0f ? a.z : b.z;
    return ret;
}

constexpr Float3 Float3::SplatX() const
{
    return Float3(x, x, x);
}

constexpr Float3 Float3::SplatY() const
{
    return Float3(y, y, y);
}

constexpr Float3 Float3::SplatZ() const
{
    return Float3(z, z, z);
}

constexpr Float3 Float3::Splat(float f)
{
    return Float3(f, f, f);
}


//////////////////////////////////////////////////////////////////////////

constexpr Float3 Float3::operator- () const
{
    return Float3(-x, -y, -z);
}

constexpr Float3 Float3::operator+ (const Float3& b) const
{
    return Float3(x + b.x, y + b.y, z + b.z);
}

constexpr Float3 Float3::operator- (const Float3& b) const
{
    return Float3(x - b.x, y - b.y, z - b.z);
}

constexpr Float3 Float3::operator* (const Float3& b) const
{
    return Float3(x * b.x, y * b.y, z * b.z);
}

Float3 Float3::operator/ (const Float3& b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b.x) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.y) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.z) > NFE_MATH_EPSILON, "Division by zero");

    return Float3(x / b.x, y / b.y, z / b.z);
}

constexpr Float3 Float3::operator* (float b) const
{
    return Float3(x * b, y * b, z * b);
}

Float3 Float3::operator/ (float b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    return Float3(x / b, y / b, z / b);
}

Float3 operator*(float a, const Float3& b)
{
    return Float3(a * b.x, a * b.y, a * b.z);
}

//////////////////////////////////////////////////////////////////////////

Float3& Float3::operator+= (const Float3& b)
{
    x += b.x;
    y += b.y;
    z += b.z;
    return *this;
}

Float3& Float3::operator-= (const Float3& b)
{
    x -= b.x;
    y -= b.y;
    z -= b.z;
    return *this;
}

Float3& Float3::operator*= (const Float3& b)
{
    x *= b.x;
    y *= b.y;
    z *= b.z;
    return *this;
}

Float3& Float3::operator/= (const Float3& b)
{
    NFE_ASSERT(Math::Abs(b.x) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.y) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.z) > NFE_MATH_EPSILON, "Division by zero");

    x /= b.x;
    y /= b.y;
    z /= b.z;
    return *this;
}

Float3& Float3::operator*= (float b)
{
    x *= b;
    y *= b;
    z *= b;
    return *this;
}

Float3& Float3::operator/= (float b)
{
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    x /= b;
    y /= b;
    z /= b;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

Float3 Float3::Floor(const Float3& v)
{
    return Float3(floorf(v.x), floorf(v.y), floorf(v.z));
}

Float3 Float3::Sqrt(const Float3& v)
{
    NFE_ASSERT(v.x >= 0.0f, "Square root of negative number");
    NFE_ASSERT(v.y >= 0.0f, "Square root of negative number");
    NFE_ASSERT(v.z >= 0.0f, "Square root of negative number");

    return Float3(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z));
}

Float3 Float3::Reciprocal(const Float3& v)
{
    NFE_ASSERT(Math::Abs(v.x) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.y) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.z) > NFE_MATH_EPSILON, "Division by zero");

    // this checks are required to avoid "potential divide by 0" warning
    return Float3(v.x != 0.0f ? 1.0f / v.x : INFINITY,
                  v.y != 0.0f ? 1.0f / v.y : INFINITY,
                  v.z != 0.0f ? 1.0f / v.z : INFINITY);
}

constexpr Float3 Float3::Min(const Float3& a, const Float3& b)
{
    return Float3(
        Math::Min<float>(a.x, b.x),
        Math::Min<float>(a.y, b.y),
        Math::Min<float>(a.z, b.z)
    );
}

constexpr Float3 Float3::Max(const Float3& a, const Float3& b)
{
    return Float3(
        Math::Max<float>(a.x, b.x),
        Math::Max<float>(a.y, b.y),
        Math::Max<float>(a.z, b.z)
    );
}

constexpr Float3 Float3::Abs(const Float3& v)
{
    return Float3(Math::Abs(v.x), Math::Abs(v.y), Math::Abs(v.z));
}

constexpr Float3 Float3::Lerp(const Float3& v1, const Float3& v2, const Float3& weight)
{
    return Float3(
        v1.x + weight.x * (v2.x - v1.x),
        v1.y + weight.y * (v2.y - v1.y),
        v1.z + weight.z * (v2.z - v1.z)
    );
}

constexpr Float3 Float3::Lerp(const Float3& v1, const Float3& v2, float weight)
{
    return Float3(
        v1.x + weight * (v2.x - v1.x),
        v1.y + weight * (v2.y - v1.y),
        v1.z + weight * (v2.z - v1.z)
    );
}

//////////////////////////////////////////////////////////////////////////

constexpr bool Float3::operator== (const Float3& b) const
{
    return (x == b.x) && (y == b.y) && (z == b.z);
}

constexpr bool Float3::operator< (const Float3& b) const
{
    return (x < b.x) && (y < b.y) && (z < b.z);
}

constexpr bool Float3::operator<= (const Float3& b) const
{
    return (x <= b.x) && (y <= b.y) && (z <= b.z);
}

constexpr bool Float3::operator> (const Float3& b) const
{
    return (x > b.x) && (y > b.y) && (z > b.z);
}

constexpr bool Float3::operator>= (const Float3& b) const
{
    return (x >= b.x) && (y >= b.y) && (z >= b.z);
}

constexpr bool Float3::operator!= (const Float3& b) const
{
    return (x != b.x) && (y != b.y) && (z != b.z);
}

constexpr bool Float3::AlmostEqual(const Float3& v1, const Float3& v2, float epsilon)
{
    return Abs(v1 - v2) < Float3::Splat(epsilon);
}

//////////////////////////////////////////////////////////////////////////

constexpr float Float3::Dot(const Float3& a, const Float3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr Float3 Float3::Cross(const Float3& a, const Float3& b)
{
    return Float3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

float Float3::Length() const
{
    return sqrtf(x * x + y * y + z * z);
}

Float3 Float3::Normalized() const
{
    const float len = Length();
    NFE_ASSERT(len > NFE_MATH_EPSILON, "Trying to normalize a vector of length 0");

    const float lenInv = 1.0f / len;
    return *this * lenInv;
}

Float3& Float3::Normalize()
{
    const float len = Length();
    NFE_ASSERT(len > NFE_MATH_EPSILON, "Trying to normalize a vector of length 0");

    const float lenInv = 1.0f / len;
    *this *= lenInv;
    return *this;
}

Float3 Float3::Reflect(const Float3& i, const Float3& n)
{
    return i - 2.0f * n * Float3::Dot(i, n);
}

} // namespace Math
} // namespace NFE