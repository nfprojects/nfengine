#pragma once

#include "Vec4f.hpp"

namespace NFE {
namespace Math {

const Vec2f Vec4f::ToVec2f() const
{
    return Vec2f{ x, y };
}

const Vec3f Vec4f::ToVec3f() const
{
    return Vec3f{ x, y, z };
}

const Vec4fU Vec4f::ToVec4fU() const
{
    return Vec4fU{ x, y, z, w };
}

const Vec4f Vec4f::SplatX() const
{
    return Swizzle<0, 0, 0, 0>();
}

const Vec4f Vec4f::SplatY() const
{
    return Swizzle<1, 1, 1, 1>();
}

const Vec4f Vec4f::SplatZ() const
{
    return Swizzle<2, 2, 2, 2>();
}

const Vec4f Vec4f::SplatW() const
{
    return Swizzle<3, 3, 3, 3>();
}

const Vec4f Vec4f::MulAndAdd(const Vec4f& a, const float b, const Vec4f& c)
{
    return MulAndAdd(a, Vec4f(b), c);
}

const Vec4f Vec4f::MulAndSub(const Vec4f& a, const float b, const Vec4f& c)
{
    return MulAndSub(a, Vec4f(b), c);
}

const Vec4f Vec4f::NegMulAndAdd(const Vec4f& a, const float b, const Vec4f& c)
{
    return NegMulAndAdd(a, Vec4f(b), c);
}

const Vec4f Vec4f::NegMulAndSub(const Vec4f& a, const float b, const Vec4f& c)
{
    return NegMulAndSub(a, Vec4f(b), c);
}

const Vec4f Vec4f::Lerp(const Vec4f& v1, const Vec4f& v2, const Vec4f& weight)
{
    return MulAndAdd(v2 - v1, weight, v1);
}

const Vec4f Vec4f::Lerp(const Vec4f& v1, const Vec4f& v2, float weight)
{
    return MulAndAdd(v2 - v1, Vec4f(weight), v1);
}

const Vec4f Vec4f::Saturate(const Vec4f& v)
{
    return Min(Vec4f(1.0f), Max(Vec4f::Zero(), v));
}

const Vec4f Vec4f::Clamp(const Vec4f& x, const Vec4f& min, const Vec4f& max)
{
    return Min(max, Max(min, x));
}

float Vec4f::SqrLength2() const
{
    return Dot2(*this, *this);
}

const Vec4f Vec4f::Normalized3() const
{
    Vec4f result = *this;
    result.Normalize3();
    return result;
}

const Vec4f Vec4f::InvNormalized(Vec4f& outInvNormalized) const
{
    const Vec4f len = Length3V();
    const Vec4f temp = Vec4f::Select<0, 0, 0, 1>(*this, len);
    const Vec4f invTemp = Vec4f::Reciprocal(temp); // [1/x, 1/y, 1/y, 1/length]

    outInvNormalized = len * invTemp;
    return (*this) * invTemp.w;
}

const Vec4f Vec4f::FastNormalized3() const
{
    Vec4f result = *this;
    result.FastNormalize3();
    return result;
}

float Vec4f::SqrLength4() const
{
    return Dot4(*this, *this);
}

const Vec4f Vec4f::Normalized4() const
{
    Vec4f result = *this;
    result.Normalize4();
    return result;
}

const Vec4f Vec4f::Reflect3(const Vec4f& i, const Vec4f& n)
{
    // return (i - 2.0f * Dot(i, n) * n);
    const Vec4f vDot = Dot3V(i, n);
    return NegMulAndAdd(vDot + vDot, n, i);
}

const Vec4f Vec4f::Orthogonalize(const Vec4f& v, const Vec4f& reference)
{
    // Gram–Schmidt process
    return Vec4f::NegMulAndAdd(Vec4f::Dot3V(v, reference), reference, v);
}

const Vec4f BipolarToUnipolar(const Vec4f& x)
{
    return Vec4f::MulAndAdd(x, VECTOR_HALVES, VECTOR_HALVES);
}

const Vec4f UnipolarToBipolar(const Vec4f& x)
{
    return Vec4f::MulAndSub(x, 2.0f, VECTOR_ONE);
}

bool Vec4f::AlmostEqual(const Vec4f& v1, const Vec4f& v2, float epsilon)
{
    return (Abs(v1 - v2) < Vec4f(epsilon)).All();
}

const VecBool4f Vec4f::IsZero() const
{
    return *this == Vec4f::Zero();
}

} // namespace Math
} // namespace NFE
