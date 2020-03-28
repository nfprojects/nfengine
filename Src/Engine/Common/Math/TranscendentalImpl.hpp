#pragma once

namespace NFE {
namespace Math {


float Cos(float x)
{
    return Sin(x + NFE_MATH_PI / 2.0f);
}

const Vec4f Cos(const Vec4f& x)
{
    return Sin(x + Vec4f(NFE_MATH_PI / 2.0f));
}

const Vec8f Cos(const Vec8f& x)
{
    return Sin(x + Vec8f(NFE_MATH_PI / 2.0f));
}

const Vec4f SinCos(const float x)
{
    const Vec4f offset(0.0f, NFE_MATH_PI / 2.0f, 0.0f, 0.0f);
    return Sin(Vec4f(x) + offset) & Vec4f::MakeMask<1, 1, 0, 0>();
}

float Tan(float x)
{
    return Sin(x) / Cos(x);
}

float Cot(float x)
{
    return Cos(x) / Sin(x);
}

float FastExp(float a)
{
    return FastExp2(a * 1.442695041f); // 1.0f/log(2.0f)
}

const Vec4f FastExp(const Vec4f& a)
{
    return FastExp2(a * 1.442695041f); // 1.0f/log(2.0f)
}

const Vec8f FastExp(const Vec8f& a)
{
    return FastExp2(a * 1.442695041f); // 1.0f/log(2.0f)
}

} // namespace Math
} // namespace NFE
