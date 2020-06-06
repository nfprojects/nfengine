#pragma once

#include "Vec2f.hpp"


namespace NFE {
namespace Math {


static_assert(sizeof(Vec2f) == 2 * sizeof(float), "Invalid Vec2f size");


constexpr Vec2f::Vec2f()
    : x(0.0f), y(0.0f)
{ }

constexpr Vec2f::Vec2f(const float* src)
    : x(src[0]), y(src[1])
{ }

constexpr Vec2f::Vec2f(float v)
    : x(v), y(v)
{ }

constexpr Vec2f::Vec2f(float x, float y)
    : x(x), y(y)
{ }

float Vec2f::Get(uint32 index) const
{
    NFE_ASSERT(index < 2, "Invalid index");
    return (&x)[index];
}

float& Vec2f::Get(uint32 index)
{
    NFE_ASSERT(index < 2, "Invalid index");
    return (&x)[index];
}

bool Vec2f::IsValid() const
{
    return Math::IsValid(x) && Math::IsValid(y);
}

//////////////////////////////////////////////////////////////////////////

template<uint32 ix, uint32 iy>
const Vec2f Vec2f::Swizzle() const
{
    static_assert(ix < 2, "Invalid X element index");
    static_assert(iy < 2, "Invalid Y element index");

    return Vec2f((&x)[ix], (&x)[iy]);
}

template<uint32 ix, uint32 iy>
constexpr const Vec2f Vec2f::Blend(const Vec2f& a, const Vec2f& b)
{
    return Vec2f(ix == 0 ? a.x : b.x, iy == 0 ? a.y : b.y);
}

template<bool changeX, bool changeY>
constexpr const Vec2f Vec2f::ChangeSign() const
{
    return Vec2f(changeX ? -x : x, changeY ? -y : y);
}

const Vec2f Vec2f::SelectBySign(const Vec2f& a, const Vec2f& b, const Vec2f& sel)
{
    return Vec2f(sel.x > 0.0f ? a.x : b.x, sel.y > 0.0f ? a.y : b.y);
}

constexpr const Vec2f Vec2f::SplatX() const
{
    return Vec2f(x, x);
}

constexpr const Vec2f Vec2f::SplatY() const
{
    return Vec2f(y, y);
}

constexpr const Vec2f Vec2f::Splat(float f)
{
    return Vec2f(f, f);
}

//////////////////////////////////////////////////////////////////////////

constexpr const Vec2f Vec2f::operator- () const
{
    return Vec2f(-x, -y);
}

constexpr const Vec2f Vec2f::operator+ (const Vec2f& b) const
{
    return Vec2f(x + b.x, y + b.y);
}

constexpr const Vec2f Vec2f::operator- (const Vec2f& b) const
{
    return Vec2f(x - b.x, y - b.y);
}

constexpr const Vec2f Vec2f::operator* (const Vec2f& b) const
{
    return Vec2f(x * b.x, y * b.y);
}

const Vec2f Vec2f::operator/ (const Vec2f& b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b.x) > FLT_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.y) > FLT_EPSILON, "Division by zero");

    return Vec2f(x / b.x, y / b.y);
}

constexpr const Vec2f Vec2f::operator* (float b) const
{
    return Vec2f(x * b, y * b);
}

const Vec2f Vec2f::operator/ (float b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b) > FLT_EPSILON, "Division by zero");

    return Vec2f(x / b, y / b);
}

const Vec2f operator*(float a, const Vec2f& b)
{
    return Vec2f(a * b.x, a * b.y);
}

//////////////////////////////////////////////////////////////////////////

Vec2f& Vec2f::operator+= (const Vec2f& b)
{
    x += b.x;
    y += b.y;
    return *this;
}

Vec2f& Vec2f::operator-= (const Vec2f& b)
{
    x -= b.x;
    y -= b.y;
    return *this;
}

Vec2f& Vec2f::operator*= (const Vec2f& b)
{
    x *= b.x;
    y *= b.y;
    return *this;
}

Vec2f& Vec2f::operator/= (const Vec2f& b)
{
    NFE_ASSERT(Math::Abs(b.x) > FLT_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.y) > FLT_EPSILON, "Division by zero");

    x /= b.x;
    y /= b.y;
    return *this;
}

Vec2f& Vec2f::operator*= (float b)
{
    x *= b;
    y *= b;
    return *this;
}

Vec2f& Vec2f::operator/= (float b)
{
    NFE_ASSERT(Math::Abs(b) > FLT_EPSILON, "Division by zero");

    x /= b;
    y /= b;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const Vec2f Vec2f::Floor(const Vec2f& v)
{
    return Vec2f(floorf(v.x), floorf(v.y));
}

const Vec2f Vec2f::Sqrt(const Vec2f& v)
{
    NFE_ASSERT(v.x >= 0.0f, "Sqrt of negative value");
    NFE_ASSERT(v.y >= 0.0f, "Sqrt of negative value");

    return Vec2f(sqrtf(v.x), sqrtf(v.y));
}

const Vec2f Vec2f::Reciprocal(const Vec2f& v)
{
    NFE_ASSERT(Math::Abs(v.x) > FLT_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.y) > FLT_EPSILON, "Division by zero");

    // this checks are required to avoid "potential divide by 0" warning
    return Vec2f(v.x != 0.0f ? 1.0f / v.x : INFINITY,
                  v.y != 0.0f ? 1.0f / v.y : INFINITY);
}

constexpr const Vec2f Vec2f::Min(const Vec2f& a, const Vec2f& b)
{
    return Vec2f(Math::Min<float>(a.x, b.x), Math::Min<float>(a.y, b.y));
}

constexpr const Vec2f Vec2f::Max(const Vec2f& a, const Vec2f& b)
{
    return Vec2f(Math::Max<float>(a.x, b.x), Math::Max<float>(a.y, b.y));
}

constexpr const Vec2f Vec2f::Abs(const Vec2f& v)
{
    return Vec2f(Math::Abs(v.x), Math::Abs(v.y));
}

constexpr const Vec2f Vec2f::Lerp(const Vec2f& v1, const Vec2f& v2, const Vec2f& weight)
{
    return Vec2f(v1.x + weight.x * (v2.x - v1.x), v1.y + weight.y * (v2.y - v1.y));
}

constexpr const Vec2f Vec2f::Lerp(const Vec2f& v1, const Vec2f& v2, float weight)
{
    return Vec2f(v1.x + weight * (v2.x - v1.x), v1.y + weight * (v2.y - v1.y));
}

//////////////////////////////////////////////////////////////////////////

constexpr bool Vec2f::operator== (const Vec2f& b) const
{
    return (x == b.x) && (y == b.y);
}

constexpr bool Vec2f::operator< (const Vec2f& b) const
{
    return (x < b.x) && (y < b.y);
}

constexpr bool Vec2f::operator<= (const Vec2f& b) const
{
    return (x <= b.x) && (y <= b.y);
}

constexpr bool Vec2f::operator> (const Vec2f& b) const
{
    return (x > b.x) && (y > b.y);
}

constexpr bool Vec2f::operator>= (const Vec2f& b) const
{
    return (x >= b.x) && (y >= b.y);
}

constexpr bool Vec2f::operator!= (const Vec2f& b) const
{
    return (x != b.x) && (y != b.y);
}

constexpr bool Vec2f::AlmostEqual(const Vec2f& v1, const Vec2f& v2, float epsilon)
{
    return Abs(v1 - v2) < Vec2f::Splat(epsilon);
}

//////////////////////////////////////////////////////////////////////////

constexpr float Vec2f::Dot(const Vec2f& a, const Vec2f& b)
{
    return a.x * b.x + a.y * b.y;
}

constexpr float Vec2f::Cross(const Vec2f& a, const Vec2f& b)
{
    return a.x * b.y - a.y * b.x;
}

float Vec2f::Length() const
{
    return sqrtf(x * x + y * y);
}

const Vec2f Vec2f::Normalized() const
{
    const float len = Length();
    NFE_ASSERT(len > FLT_EPSILON, "Division by zero");

    const float lenInv = 1.0f / len;
    return *this * lenInv;
}

Vec2f& Vec2f::Normalize()
{
    const float len = Length();
    NFE_ASSERT(len > FLT_EPSILON, "Division by zero");

    const float lenInv = 1.0f / len;
    *this *= lenInv;
    return *this;
}

} // namespace Math
} // namespace NFE
