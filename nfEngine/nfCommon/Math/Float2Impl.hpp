/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Float2 class definition.
 */

#pragma once

#include "Float2.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace Math {


static_assert(sizeof(Float2) == 2 * sizeof(float), "Invalid Float2 size");


constexpr Float2::Float2()
    : x(0.0f), y(0.0f)
{ }

constexpr Float2::Float2(const float* src)
    : x(src[0]), y(src[1])
{ }

constexpr Float2::Float2(float x, float y)
    : x(x), y(y)
{ }

float Float2::Get(uint32 index) const
{
    NFE_ASSERT(index < 2, "Invalid vector index");
    return (&x)[index];
}

float& Float2::Get(uint32 index)
{
    NFE_ASSERT(index < 2, "Invalid vector index");
    return (&x)[index];
}

//////////////////////////////////////////////////////////////////////////

template<uint32 ix, uint32 iy>
Float2 Float2::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");

    return Float2((&x)[ix], (&x)[iy]);
}

template<uint32 ix, uint32 iy>
constexpr Float2 Float2::Blend(const Float2& a, const Float2& b)
{
    return Float2(ix == 0 ? a.x : b.x, iy == 0 ? a.y : b.y);
}

template<bool negX, bool negY>
constexpr Float2 Float2::ChangeSign() const
{
    return Float2(negX ? -x : x, negY ? -y : y);
}

Float2 Float2::SelectBySign(const Float2& a, const Float2& b, const Float2& sel)
{
    return Float2(sel.x > 0.0f ? a.x : b.x, sel.y > 0.0f ? a.y : b.y);
}

constexpr Float2 Float2::SplatX() const
{
    return Float2(x, x);
}

constexpr Float2 Float2::SplatY() const
{
    return Float2(y, y);
}

constexpr Float2 Float2::Splat(float f)
{
    return Float2(f, f);
}


//////////////////////////////////////////////////////////////////////////

constexpr Float2 Float2::operator- () const
{
    return Float2(-x, -y);
}

constexpr Float2 Float2::operator+ (const Float2& b) const
{
    return Float2(x + b.x, y + b.y);
}

constexpr Float2 Float2::operator- (const Float2& b) const
{
    return Float2(x - b.x, y - b.y);
}

constexpr Float2 Float2::operator* (const Float2& b) const
{
    return Float2(x * b.x, y * b.y);
}

Float2 Float2::operator/ (const Float2& b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b.x) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.y) > NFE_MATH_EPSILON, "Division by zero");

    return Float2(x / b.x, y / b.y);
}

constexpr Float2 Float2::operator* (float b) const
{
    return Float2(x * b, y * b);
}

Float2 Float2::operator/ (float b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    return Float2(x / b, y / b);
}

Float2 operator*(float a, const Float2& b)
{
    return Float2(a * b.x, a * b.y);
}

//////////////////////////////////////////////////////////////////////////

Float2& Float2::operator+= (const Float2& b)
{
    x += b.x;
    y += b.y;
    return *this;
}

Float2& Float2::operator-= (const Float2& b)
{
    x -= b.x;
    y -= b.y;
    return *this;
}

Float2& Float2::operator*= (const Float2& b)
{
    x *= b.x;
    y *= b.y;
    return *this;
}

Float2& Float2::operator/= (const Float2& b)
{
    NFE_ASSERT(Math::Abs(b.x) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.y) > NFE_MATH_EPSILON, "Division by zero");

    x /= b.x;
    y /= b.y;
    return *this;
}

Float2& Float2::operator*= (float b)
{
    x *= b;
    y *= b;
    return *this;
}

Float2& Float2::operator/= (float b)
{
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    x /= b;
    y /= b;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

Float2 Float2::Floor(const Float2& v)
{
    return Float2(floorf(v.x), floorf(v.y));
}

Float2 Float2::Sqrt(const Float2& v)
{
    NFE_ASSERT(v.x >= 0.0f, "Square root of negative number");
    NFE_ASSERT(v.y >= 0.0f, "Square root of negative number");

    return Float2(sqrtf(v.x), sqrtf(v.y));
}

Float2 Float2::Reciprocal(const Float2& v)
{
    NFE_ASSERT(Math::Abs(v.x) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.y) > NFE_MATH_EPSILON, "Division by zero");

    // this checks are required to avoid "potential divide by 0" warning
    return Float2(v.x != 0.0f ? 1.0f / v.x : INFINITY,
                  v.y != 0.0f ? 1.0f / v.y : INFINITY);
}

constexpr Float2 Float2::Min(const Float2& a, const Float2& b)
{
    return Float2(Math::Min<float>(a.x, b.x), Math::Min<float>(a.y, b.y));
}

constexpr Float2 Float2::Max(const Float2& a, const Float2& b)
{
    return Float2(Math::Max<float>(a.x, b.x), Math::Max<float>(a.y, b.y));
}

constexpr Float2 Float2::Abs(const Float2& v)
{
    return Float2(Math::Abs(v.x), Math::Abs(v.y));
}

constexpr Float2 Float2::Lerp(const Float2& v1, const Float2& v2, const Float2& weight)
{
    return Float2(v1.x + weight.x * (v2.x - v1.x), v1.y + weight.y * (v2.y - v1.y));
}

constexpr Float2 Float2::Lerp(const Float2& v1, const Float2& v2, float weight)
{
    return Float2(v1.x + weight * (v2.x - v1.x), v1.y + weight * (v2.y - v1.y));
}

//////////////////////////////////////////////////////////////////////////

constexpr bool Float2::operator== (const Float2& b) const
{
    return (x == b.x) && (y == b.y);
}

constexpr bool Float2::operator< (const Float2& b) const
{
    return (x < b.x) && (y < b.y);
}

constexpr bool Float2::operator<= (const Float2& b) const
{
    return (x <= b.x) && (y <= b.y);
}

constexpr bool Float2::operator> (const Float2& b) const
{
    return (x > b.x) && (y > b.y);
}

constexpr bool Float2::operator>= (const Float2& b) const
{
    return (x >= b.x) && (y >= b.y);
}

constexpr bool Float2::operator!= (const Float2& b) const
{
    return (x != b.x) && (y != b.y);
}

constexpr bool Float2::AlmostEqual(const Float2& v1, const Float2& v2, float epsilon)
{
    return Abs(v1 - v2) < Float2::Splat(epsilon);
}

//////////////////////////////////////////////////////////////////////////

constexpr float Float2::Dot(const Float2& a, const Float2& b)
{
    return a.x * b.x + a.y * b.y;
}

constexpr float Float2::Cross(const Float2& a, const Float2& b)
{
    return a.x * b.y - a.y * b.x;
}

float Float2::Length() const
{
    return sqrtf(x * x + y * y);
}

Float2 Float2::Normalized() const
{
    const float len = Length();
    NFE_ASSERT(len > NFE_MATH_EPSILON, "Trying to normalize a vector of length 0");

    const float lenInv = 1.0f / len;
    return *this * lenInv;
}

Float2& Float2::Normalize()
{
    const float len = Length();
    NFE_ASSERT(len > NFE_MATH_EPSILON, "Trying to normalize a vector of length 0");

    const float lenInv = 1.0f / len;
    *this *= lenInv;
    return *this;
}

Float2 Float2::Reflect(const Float2& i, const Float2& n)
{
    return i - 2.0f * n * Float2::Dot(i, n);
}

} // namespace Math
} // namespace NFE