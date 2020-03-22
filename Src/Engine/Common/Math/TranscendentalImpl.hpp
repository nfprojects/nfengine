#pragma once

namespace NFE {
namespace Math {


float Cos(float x)
{
    return Sin(x + NFE_MATH_PI / 2.0f);
}

const Vector4 Cos(const Vector4& x)
{
    return Sin(x + Vector4(NFE_MATH_PI / 2.0f));
}

const Vector8 Cos(const Vector8& x)
{
    return Sin(x + Vector8(NFE_MATH_PI / 2.0f));
}

const Vector4 SinCos(const float x)
{
    const Vector4 offset(0.0f, NFE_MATH_PI / 2.0f, 0.0f, 0.0f);
    return Sin(Vector4(x) + offset) & Vector4::MakeMask<1, 1, 0, 0>();
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

const Vector4 FastExp(const Vector4& a)
{
    return FastExp2(a * 1.442695041f); // 1.0f/log(2.0f)
}

const Vector8 FastExp(const Vector8& a)
{
    return FastExp2(a * 1.442695041f); // 1.0f/log(2.0f)
}

} // namespace Math
} // namespace NFE
