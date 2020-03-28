#pragma once

namespace NFE {
namespace Math {

bool Vec8f::AlmostEqual(const Vec8f& v1, const Vec8f& v2, float epsilon)
{
    return (Abs(v1 - v2) < Vec8f(epsilon)).All();
}

const Vec8f Vec8f::MulAndAdd(const Vec8f& a, const float b, const Vec8f& c)
{
    return MulAndAdd(a, Vec8f(b), c);
}

const Vec8f Vec8f::MulAndSub(const Vec8f& a, const float b, const Vec8f& c)
{
    return MulAndSub(a, Vec8f(b), c);
}

const Vec8f Vec8f::NegMulAndAdd(const Vec8f& a, const float b, const Vec8f& c)
{
    return NegMulAndAdd(a, Vec8f(b), c);
}

const Vec8f Vec8f::NegMulAndSub(const Vec8f& a, const float b, const Vec8f& c)
{
    return NegMulAndSub(a, Vec8f(b), c);
}

const Vec8f Vec8f::Lerp(const Vec8f& v1, const Vec8f& v2, const Vec8f& weight)
{
    return MulAndAdd(v2 - v1, weight, v1);
}

const Vec8f Vec8f::Lerp(const Vec8f& v1, const Vec8f& v2, float weight)
{
    return MulAndAdd(v2 - v1, weight, v1);
}

const Vec8f Vec8f::Clamped(const Vec8f& min, const Vec8f& max) const
{
    return Min(max, Max(min, *this));
}

bool Vec8f::IsValid() const
{
    return !IsNaN() && !IsInfinite();
}

} // namespace Math
} // namespace NFE
