/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Float4 class definition.
 */

#pragma once

#include "Float4.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace Math {


static_assert(sizeof(Float4) == 4 * sizeof(float), "Invalid Float4 size");


constexpr Float4::Float4()
    : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
{ }

constexpr Float4::Float4(const float* src)
    : x(src[0]), y(src[1]), z(src[2]), w(src[3])
{ }

constexpr Float4::Float4(const Float2& src)
    : x(src.x), y(src.y), z(0.0f), w(0.0f)
{ }

constexpr Float4::Float4(const Float3& src)
    : x(src.x), y(src.y), z(src.z), w(0.0f)
{ }

constexpr Float4::Float4(float x, float y, float z, float w)
    : x(x), y(y), z(z), w(w)
{ }

float Float4::Get(uint32 index) const
{
    NFE_ASSERT(index < 4, "Invalid vector index");
    return (&x)[index];
}

float& Float4::Get(uint32 index)
{
    NFE_ASSERT(index < 4, "Invalid vector index");
    return (&x)[index];
}

Float4::operator Float2() const
{
    return Float2(x, y);
}

Float4::operator Float3() const
{
    return Float3(x, y, z);
}

//////////////////////////////////////////////////////////////////////////

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
Float4 Float4::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return Float4((&x)[ix], (&x)[iy], (&x)[iz], (&x)[iw]);
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
constexpr Float4 Float4::Blend(const Float4& a, const Float4& b)
{
    return Float4(ix == 0 ? a.x : b.x,
                  iy == 0 ? a.y : b.y,
                  iz == 0 ? a.z : b.z,
                  iw == 0 ? a.w : b.w);
}

template<bool negX, bool negY, bool negZ, bool negW>
constexpr Float4 Float4::ChangeSign() const
{
    return Float4(
        negX ? -x : x,
        negY ? -y : y,
        negZ ? -z : z,
        negW ? -w : w
    );
}

Float4 Float4::SelectBySign(const Float4& a, const Float4& b, const Float4& sel)
{
    Float4 ret;
    ret.x = sel.x > 0.0f ? a.x : b.x;
    ret.y = sel.y > 0.0f ? a.y : b.y;
    ret.z = sel.z > 0.0f ? a.z : b.z;
    ret.w = sel.w > 0.0f ? a.w : b.w;
    return ret;
}

constexpr Float4 Float4::SplatX() const
{
    return Float4(x, x, x, x);
}

constexpr Float4 Float4::SplatY() const
{
    return Float4(y, y, y, y);
}

constexpr Float4 Float4::SplatZ() const
{
    return Float4(z, z, z, z);
}

constexpr Float4 Float4::SplatW() const
{
    return Float4(w, w, w, w);
}

constexpr Float4 Float4::Splat(float f)
{
    return Float4(f, f, f, f);
}


//////////////////////////////////////////////////////////////////////////

constexpr Float4 Float4::operator- () const
{
    return Float4(-x, -y, -z, -w);
}

constexpr Float4 Float4::operator+ (const Float4& b) const
{
    return Float4(x + b.x, y + b.y, z + b.z, w + b.w);
}

constexpr Float4 Float4::operator- (const Float4& b) const
{
    return Float4(x - b.x, y - b.y, z - b.z, w - b.w);
}

constexpr Float4 Float4::operator* (const Float4& b) const
{
    return Float4(x * b.x, y * b.y, z * b.z, w * b.w);
}

Float4 Float4::operator/ (const Float4& b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b.x) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.y) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.z) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(b.w) > NFE_MATH_EPSILON, "Division by zero");

    return Float4(x / b.x, y / b.y, z / b.z, w / b.w);
}

constexpr Float4 Float4::operator* (float b) const
{
    return Float4(x * b, y * b, z * b, w * b);
}

Float4 Float4::operator/ (float b) const
{
    // TODO make it constexpr
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    return Float4(x / b, y / b, z / b, w / b);
}

Float4 operator*(float a, const Float4& b)
{
    return Float4(a * b.x, a * b.y, a * b.z, a * b.w);
}

//////////////////////////////////////////////////////////////////////////

Float4& Float4::operator+= (const Float4& b)
{
    x += b.x;
    y += b.y;
    z += b.z;
    w += b.w;
    return *this;
}

Float4& Float4::operator-= (const Float4& b)
{
    x -= b.x;
    y -= b.y;
    z -= b.z;
    w -= b.w;
    return *this;
}

Float4& Float4::operator*= (const Float4& b)
{
    x *= b.x;
    y *= b.y;
    z *= b.z;
    w *= b.w;
    return *this;
}

Float4& Float4::operator/= (const Float4& b)
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

Float4& Float4::operator*= (float b)
{
    x *= b;
    y *= b;
    z *= b;
    w *= b;
    return *this;
}

Float4& Float4::operator/= (float b)
{
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    x /= b;
    y /= b;
    z /= b;
    w /= b;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

Float4 Float4::Floor(const Float4& v)
{
    return Float4(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

Float4 Float4::Sqrt(const Float4& v)
{
    NFE_ASSERT(v.x >= 0.0f, "Square root of negative number");
    NFE_ASSERT(v.y >= 0.0f, "Square root of negative number");
    NFE_ASSERT(v.z >= 0.0f, "Square root of negative number");
    NFE_ASSERT(v.w >= 0.0f, "Square root of negative number");

    return Float4(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z), sqrtf(v.w));
}

Float4 Float4::Reciprocal(const Float4& v)
{
    NFE_ASSERT(Math::Abs(v.x) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.y) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.z) > NFE_MATH_EPSILON, "Division by zero");
    NFE_ASSERT(Math::Abs(v.w) > NFE_MATH_EPSILON, "Division by zero");

    // this checks are required to avoid "potential divide by 0" warning
    return Float4(v.x != 0.0f ? 1.0f / v.x : INFINITY,
                  v.y != 0.0f ? 1.0f / v.y : INFINITY,
                  v.z != 0.0f ? 1.0f / v.z : INFINITY,
                  v.w != 0.0f ? 1.0f / v.w : INFINITY);
}

constexpr Float4 Float4::Min(const Float4& a, const Float4& b)
{
    return Float4(
        Math::Min<float>(a.x, b.x),
        Math::Min<float>(a.y, b.y),
        Math::Min<float>(a.z, b.z),
        Math::Min<float>(a.w, b.w)
    );
}

constexpr Float4 Float4::Max(const Float4& a, const Float4& b)
{
    return Float4(
        Math::Max<float>(a.x, b.x),
        Math::Max<float>(a.y, b.y),
        Math::Max<float>(a.z, b.z),
        Math::Max<float>(a.w, b.w)
    );
}

constexpr Float4 Float4::Abs(const Float4& v)
{
    return Float4(Math::Abs(v.x), Math::Abs(v.y), Math::Abs(v.z), Math::Abs(v.w));
}

constexpr Float4 Float4::Lerp(const Float4& v1, const Float4& v2, const Float4& weight)
{
    return Float4(
        v1.x + weight.x * (v2.x - v1.x),
        v1.y + weight.y * (v2.y - v1.y),
        v1.z + weight.z * (v2.z - v1.z),
        v1.w + weight.w * (v2.w - v1.w)
    );
}

constexpr Float4 Float4::Lerp(const Float4& v1, const Float4& v2, float weight)
{
    return Float4(
        v1.x + weight * (v2.x - v1.x),
        v1.y + weight * (v2.y - v1.y),
        v1.z + weight * (v2.z - v1.z),
        v1.w + weight * (v2.w - v1.w)
    );
}

//////////////////////////////////////////////////////////////////////////

constexpr bool Float4::operator== (const Float4& b) const
{
    return ((x == b.x) && (y == b.y)) && ((z == b.z) && (w == b.w));
}

constexpr bool Float4::operator< (const Float4& b) const
{
    return ((x < b.x) && (y < b.y)) && ((z < b.z) && (w < b.w));
}

constexpr bool Float4::operator<= (const Float4& b) const
{
    return ((x <= b.x) && (y <= b.y)) && ((z <= b.z) && (w <= b.w));
}

constexpr bool Float4::operator> (const Float4& b) const
{
    return ((x > b.x) && (y > b.y)) && ((z > b.z) && (w > b.w));
}

constexpr bool Float4::operator>= (const Float4& b) const
{
    return ((x >= b.x) && (y >= b.y)) && ((z >= b.z) && (w >= b.w));
}

constexpr bool Float4::operator!= (const Float4& b) const
{
    return ((x != b.x) && (y != b.y)) && ((z != b.z) && (w != b.w));
}

constexpr bool Float4::AlmostEqual(const Float4& v1, const Float4& v2, float epsilon)
{
    return Abs(v1 - v2) < Float4::Splat(epsilon);
}

//////////////////////////////////////////////////////////////////////////

constexpr float Float4::Dot(const Float4& a, const Float4& b)
{
    return (a.x * b.x + a.y * b.y) + (a.z * b.z + a.w * b.w);
}

constexpr Float4 Float4::Cross(const Float4& a, const Float4& b, const Float4& c)
{
    return Float4(
         a.y * (b.z * c.w - c.z * b.w) - a.z * (b.y * c.w - c.y * b.w) + a.w * (b.y * c.z - c.y * b.z),
        -a.x * (b.z * c.w - c.z * b.w) + a.z * (b.x * c.w - c.x * b.w) - a.w * (b.x * c.z - c.x * b.z),
         a.x * (b.y * c.w - c.y * b.w) - a.y * (b.x * c.w - c.x * b.w) + a.w * (b.x * c.y - c.x * b.y),
        -a.x * (b.y * c.z - c.y * b.z) + a.y * (b.x * c.z - c.x * b.z) - a.z * (b.x * c.y - c.x * b.y)
    );
}

float Float4::Length() const
{
    return sqrtf((x * x + y * y) + (z * z + w * w));
}

Float4 Float4::Normalized() const
{
    const float len = Length();
    NFE_ASSERT(len > NFE_MATH_EPSILON, "Trying to normalize a vector of length 0");

    const float lenInv = 1.0f / len;
    return *this * lenInv;
}

Float4& Float4::Normalize()
{
    const float len = Length();
    NFE_ASSERT(len > NFE_MATH_EPSILON, "Trying to normalize a vector of length 0");

    const float lenInv = 1.0f / len;
    *this *= lenInv;
    return *this;
}

Float4 Float4::Reflect(const Float4& i, const Float4& n)
{
    return i - 2.0f * n * Float4::Dot(i, n);
}

} // namespace Math
} // namespace NFE