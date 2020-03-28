/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Vec4fU class definition.
 */

#pragma once

#include "Vec4fU.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace Math {


static_assert(sizeof(Vec4fU) == 4 * sizeof(float), "Invalid Vec4fU size");


constexpr Vec4fU::Vec4fU()
    : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
{ }

constexpr Vec4fU::Vec4fU(const float* src)
    : x(src[0]), y(src[1]), z(src[2]), w(src[3])
{ }

constexpr Vec4fU::Vec4fU(const Vec2f& src)
    : x(src.x), y(src.y), z(0.0f), w(0.0f)
{ }

constexpr Vec4fU::Vec4fU(const Vec3f& src)
    : x(src.x), y(src.y), z(src.z), w(0.0f)
{ }

constexpr Vec4fU::Vec4fU(float x, float y, float z, float w)
    : x(x), y(y), z(z), w(w)
{ }

float Vec4fU::Get(uint32 index) const
{
    NFE_ASSERT(index < 4, "Invalid vector index");
    return (&x)[index];
}

float& Vec4fU::Get(uint32 index)
{
    NFE_ASSERT(index < 4, "Invalid vector index");
    return (&x)[index];
}

Vec4fU::operator Vec2f() const
{
    return Vec2f(x, y);
}

Vec4fU::operator Vec3f() const
{
    return Vec3f(x, y, z);
}

//////////////////////////////////////////////////////////////////////////

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
Vec4fU Vec4fU::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return Vec4fU((&x)[ix], (&x)[iy], (&x)[iz], (&x)[iw]);
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
constexpr Vec4fU Vec4fU::Blend(const Vec4fU& a, const Vec4fU& b)
{
    return Vec4fU(ix == 0 ? a.x : b.x,
                  iy == 0 ? a.y : b.y,
                  iz == 0 ? a.z : b.z,
                  iw == 0 ? a.w : b.w);
}

template<bool negX, bool negY, bool negZ, bool negW>
constexpr Vec4fU Vec4fU::ChangeSign() const
{
    return Vec4fU(
        negX ? -x : x,
        negY ? -y : y,
        negZ ? -z : z,
        negW ? -w : w
    );
}

Vec4fU Vec4fU::SelectBySign(const Vec4fU& a, const Vec4fU& b, const Vec4fU& sel)
{
    Vec4fU ret;
    ret.x = sel.x > 0.0f ? a.x : b.x;
    ret.y = sel.y > 0.0f ? a.y : b.y;
    ret.z = sel.z > 0.0f ? a.z : b.z;
    ret.w = sel.w > 0.0f ? a.w : b.w;
    return ret;
}

constexpr Vec4fU Vec4fU::SplatX() const
{
    return Vec4fU(x, x, x, x);
}

constexpr Vec4fU Vec4fU::SplatY() const
{
    return Vec4fU(y, y, y, y);
}

constexpr Vec4fU Vec4fU::SplatZ() const
{
    return Vec4fU(z, z, z, z);
}

constexpr Vec4fU Vec4fU::SplatW() const
{
    return Vec4fU(w, w, w, w);
}

constexpr Vec4fU Vec4fU::Splat(float f)
{
    return Vec4fU(f, f, f, f);
}


//////////////////////////////////////////////////////////////////////////

constexpr Vec4fU Vec4fU::operator- () const
{
    return Vec4fU(-x, -y, -z, -w);
}

constexpr Vec4fU Vec4fU::operator+ (const Vec4fU& b) const
{
    return Vec4fU(x + b.x, y + b.y, z + b.z, w + b.w);
}

constexpr Vec4fU Vec4fU::operator- (const Vec4fU& b) const
{
    return Vec4fU(x - b.x, y - b.y, z - b.z, w - b.w);
}

constexpr Vec4fU Vec4fU::operator* (const Vec4fU& b) const
{
    return Vec4fU(x * b.x, y * b.y, z * b.z, w * b.w);
}

Vec4fU Vec4fU::operator/ (const Vec4fU& b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b.x) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.y) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.z) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.w) > NFE_MATH_EPSILON, "Division by zero");

    return Vec4fU(x / b.x, y / b.y, z / b.z, w / b.w);
}

constexpr Vec4fU Vec4fU::operator* (float b) const
{
    return Vec4fU(x * b, y * b, z * b, w * b);
}

Vec4fU Vec4fU::operator/ (float b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    return Vec4fU(x / b, y / b, z / b, w / b);
}

Vec4fU operator*(float a, const Vec4fU& b)
{
    return Vec4fU(a * b.x, a * b.y, a * b.z, a * b.w);
}

//////////////////////////////////////////////////////////////////////////

Vec4fU& Vec4fU::operator+= (const Vec4fU& b)
{
    x += b.x;
    y += b.y;
    z += b.z;
    w += b.w;
    return *this;
}

Vec4fU& Vec4fU::operator-= (const Vec4fU& b)
{
    x -= b.x;
    y -= b.y;
    z -= b.z;
    w -= b.w;
    return *this;
}

Vec4fU& Vec4fU::operator*= (const Vec4fU& b)
{
    x *= b.x;
    y *= b.y;
    z *= b.z;
    w *= b.w;
    return *this;
}

Vec4fU& Vec4fU::operator/= (const Vec4fU& b)
{
    NFE_ASSERT(Math::Abs(b.x) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.y) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.z) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.w) > NFE_MATH_EPSILON, "Division by zero");

    x /= b.x;
    y /= b.y;
    z /= b.z;
    w /= b.w;
    return *this;
}

Vec4fU& Vec4fU::operator*= (float b)
{
    x *= b;
    y *= b;
    z *= b;
    w *= b;
    return *this;
}

Vec4fU& Vec4fU::operator/= (float b)
{
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    x /= b;
    y /= b;
    z /= b;
    w /= b;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

Vec4fU Vec4fU::Floor(const Vec4fU& v)
{
    return Vec4fU(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

Vec4fU Vec4fU::Sqrt(const Vec4fU& v)
{
    NFE_ASSERT(v.x >= 0.0f, "Square root of negative number");
    NFE_ASSERT(v.y >= 0.0f, "Square root of negative number");
    NFE_ASSERT(v.z >= 0.0f, "Square root of negative number");
    NFE_ASSERT(v.w >= 0.0f, "Square root of negative number");

    return Vec4fU(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z), sqrtf(v.w));
}

Vec4fU Vec4fU::Reciprocal(const Vec4fU& v)
{
    NFE_ASSERT(Math::Abs(v.x) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.y) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.z) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.w) > NFE_MATH_EPSILON, "Division by zero");

    // this checks are required to avoid "potential divide by 0" warning
    return Vec4fU(v.x != 0.0f ? 1.0f / v.x : INFINITY,
                  v.y != 0.0f ? 1.0f / v.y : INFINITY,
                  v.z != 0.0f ? 1.0f / v.z : INFINITY,
                  v.w != 0.0f ? 1.0f / v.w : INFINITY);
}

constexpr Vec4fU Vec4fU::Min(const Vec4fU& a, const Vec4fU& b)
{
    return Vec4fU(
        Math::Min<float>(a.x, b.x),
        Math::Min<float>(a.y, b.y),
        Math::Min<float>(a.z, b.z),
        Math::Min<float>(a.w, b.w)
    );
}

constexpr Vec4fU Vec4fU::Max(const Vec4fU& a, const Vec4fU& b)
{
    return Vec4fU(
        Math::Max<float>(a.x, b.x),
        Math::Max<float>(a.y, b.y),
        Math::Max<float>(a.z, b.z),
        Math::Max<float>(a.w, b.w)
    );
}

constexpr Vec4fU Vec4fU::Abs(const Vec4fU& v)
{
    return Vec4fU(Math::Abs(v.x), Math::Abs(v.y), Math::Abs(v.z), Math::Abs(v.w));
}

constexpr Vec4fU Vec4fU::Lerp(const Vec4fU& v1, const Vec4fU& v2, const Vec4fU& weight)
{
    return Vec4fU(
        v1.x + weight.x * (v2.x - v1.x),
        v1.y + weight.y * (v2.y - v1.y),
        v1.z + weight.z * (v2.z - v1.z),
        v1.w + weight.w * (v2.w - v1.w)
    );
}

constexpr Vec4fU Vec4fU::Lerp(const Vec4fU& v1, const Vec4fU& v2, float weight)
{
    return Vec4fU(
        v1.x + weight * (v2.x - v1.x),
        v1.y + weight * (v2.y - v1.y),
        v1.z + weight * (v2.z - v1.z),
        v1.w + weight * (v2.w - v1.w)
    );
}

//////////////////////////////////////////////////////////////////////////

constexpr bool Vec4fU::operator== (const Vec4fU& b) const
{
    return ((x == b.x) && (y == b.y)) && ((z == b.z) && (w == b.w));
}

constexpr bool Vec4fU::operator< (const Vec4fU& b) const
{
    return ((x < b.x) && (y < b.y)) && ((z < b.z) && (w < b.w));
}

constexpr bool Vec4fU::operator<= (const Vec4fU& b) const
{
    return ((x <= b.x) && (y <= b.y)) && ((z <= b.z) && (w <= b.w));
}

constexpr bool Vec4fU::operator> (const Vec4fU& b) const
{
    return ((x > b.x) && (y > b.y)) && ((z > b.z) && (w > b.w));
}

constexpr bool Vec4fU::operator>= (const Vec4fU& b) const
{
    return ((x >= b.x) && (y >= b.y)) && ((z >= b.z) && (w >= b.w));
}

constexpr bool Vec4fU::operator!= (const Vec4fU& b) const
{
    return ((x != b.x) && (y != b.y)) && ((z != b.z) && (w != b.w));
}

constexpr bool Vec4fU::AlmostEqual(const Vec4fU& v1, const Vec4fU& v2, float epsilon)
{
    return Abs(v1 - v2) < Vec4fU::Splat(epsilon);
}

//////////////////////////////////////////////////////////////////////////

constexpr float Vec4fU::Dot(const Vec4fU& a, const Vec4fU& b)
{
    return (a.x * b.x + a.y * b.y) + (a.z * b.z + a.w * b.w);
}

constexpr Vec4fU Vec4fU::Cross(const Vec4fU& a, const Vec4fU& b, const Vec4fU& c)
{
    return Vec4fU(
         a.y * (b.z * c.w - c.z * b.w) - a.z * (b.y * c.w - c.y * b.w) + a.w * (b.y * c.z - c.y * b.z),
        -a.x * (b.z * c.w - c.z * b.w) + a.z * (b.x * c.w - c.x * b.w) - a.w * (b.x * c.z - c.x * b.z),
         a.x * (b.y * c.w - c.y * b.w) - a.y * (b.x * c.w - c.x * b.w) + a.w * (b.x * c.y - c.x * b.y),
        -a.x * (b.y * c.z - c.y * b.z) + a.y * (b.x * c.z - c.x * b.z) - a.z * (b.x * c.y - c.x * b.y)
    );
}

float Vec4fU::Length() const
{
    return sqrtf((x * x + y * y) + (z * z + w * w));
}

Vec4fU Vec4fU::Normalized() const
{
    const float len = Length();
    NFE_ASSERT(len > NFE_MATH_EPSILON, "Trying to normalize a vector of length 0");

    const float lenInv = 1.0f / len;
    return *this * lenInv;
}

Vec4fU& Vec4fU::Normalize()
{
    const float len = Length();
    NFE_ASSERT(len > NFE_MATH_EPSILON, "Trying to normalize a vector of length 0");

    const float lenInv = 1.0f / len;
    *this *= lenInv;
    return *this;
}

Vec4fU Vec4fU::Reflect(const Vec4fU& i, const Vec4fU& n)
{
    return i - 2.0f * n * Vec4fU::Dot(i, n);
}

} // namespace Math
} // namespace NFE